#include "MatchingEngine.hpp"
#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include <fmt/format.h>

#include <cassert>

namespace exch {

MatchingEngine::MatchingEngine(ExchangeConfig cfg, asio::io_context &ctx)
    : cfg{cfg}
    , incomingChannel{std::make_shared<client_send_channel_t>(ctx, 4096)}
{}

asio::awaitable<void> MatchingEngine::run() {
    co_await handleClientMessages();
}

asio::awaitable<EnginePort> MatchingEngine::connect(asio::io_context &ctx) {
    ClientId id = nextClientId++;

    clientChannels.insert({id,std::make_shared<server_send_channel_t>(ctx, 8)});

    assert(nextClientId == clientChannels.size() && "client Id should be a map into clientChannels");

    co_return EnginePort{id, incomingChannel, clientChannels[id]};
}

asio::awaitable<void> MatchingEngine::handleClientMessages() {
    while(true)
    {
        std::pair<ClientId, order::ClientOutboundMessages> clientMsg = co_await incomingChannel->async_receive(asio::use_awaitable);

        auto handleOrder = [&](const order::Order &order) -> asio::awaitable<void> {
            ClientId cid = clientMsg.first;
            assert(cid < clientChannels.size());

            order::Id orderId = nextOrderId++;

            co_await clientChannels.at(cid)->async_send(asio::error_code{}, order::Ack{
                .clientId = order.foreignOrderId,
                .vendorId = orderId
            }, asio::use_awaitable);

            auto executions= book.place(order::AckedOrder{order, cid});

            for(auto &&exec : executions)
            {
                assert(exec.side1.side != exec.side2.side && "trades should be on opposite sides");
                auto notifySell = [&]() {
                    assert(clientChannels.contains(exec.side1.clientId) && "client id not found");
                    clientChannels.at(exec.side1.clientId)->async_send(asio::error_code{}, order::Fill{
                        exec.side1.exchangeId,
                        exec.execPrice,
                        exec.execQty
                    }, asio::detached);
                };
                auto notifyBuy = [&]() {
                    assert(clientChannels.contains(exec.side2.clientId) && "client id not found");
                    clientChannels.at(exec.side2.clientId)->async_send(asio::error_code{}, order::Fill{
                        exec.side2.exchangeId,
                        exec.execPrice,
                        exec.execQty
                    }, asio::detached);
                };

                // @todo: fire these at the same time!
                // one client shouldn't here about the trade before another

                notifySell();
                notifyBuy();
            }
        };

        co_await std::visit(handleOrder,
            clientMsg.second 
        );

    }
}

}