#include "MatchingEngine.hpp"
#include "EnginePort.hpp"
#include "OrderTypes.hpp"

#include <cassert>

namespace exch {

MatchingEngine::MatchingEngine(ExchangeConfig cfg)
    : cfg{cfg}
{}

asio::awaitable<void> MatchingEngine::run() {
    co_await handleClientMessages();
}

EnginePort MatchingEngine::connect() {
    ClientId id = nextClientId++;

    clientChannels.insert({id,std::make_shared<server_send_channel_t>()});

    assert(nextClientId == clientChannels.size() && "client Id should be a map into clientChannels");

    return EnginePort{id, incomingChannel, clientChannels[id]};
}

asio::awaitable<void> MatchingEngine::handleClientMessages() {
    while(true)
    {
        std::pair<ClientId, order::ClientOutboundMessages> clientMsg = co_await incomingChannel->receive();

        auto handleOrder = [&](const order::Order &order){
            ClientId cid = order.clientId;
            assert(cid < clientChannels.size());

            order::Id orderId = nextOrderId++;

            clientChannels[cid]->send(order::Ack{
                .clientId = order.clientId,
                .vendorId = orderId
            });

            auto executions= book.place(order, orderId);

            for(auto &&exec : executions)
            {
                // @todo: fire these at the same time!
                // one client shouldn't here about the trade before another
                auto notifySell = [&](){
                    clientChannels[exec.sellOrder.clientId]->send(order::Fill{
                        exec.sellId,
                        exec.execPrice,
                        exec.execQty
                    });
                };
                auto notifyBuy = [&](){
                    clientChannels[exec.buyOrder.clientId]->send(order::Fill{
                        exec.buyId,
                        exec.execPrice,
                        exec.execQty
                    });
                };

                notifySell();
                notifyBuy();
            }
        };

        std::visit(handleOrder,
            clientMsg.second 
        );

    }
}

}