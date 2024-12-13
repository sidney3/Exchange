#pragma once
#include "ExchangeConfig.hpp"
#include "Types.hpp"
#include <asio.hpp>
#include <Channel.hpp>
#include <OrderTypes.hpp>
#include <EnginePort.hpp>
#include <OrderBook.hpp>
#include <asio/experimental/coro.hpp>

namespace exch {

class MatchingEngine
{
public:
    explicit MatchingEngine(ExchangeConfig cfg);
    [[nodiscard]] asio::awaitable<void> run();
private:
    ExchangeConfig cfg;
    OrderBook book{};
    order::Id nextOrderId = 0;

    FlatMap<order::Id, ClientId> orderOwner;
    /*
        Communication with clients 
    */
    using client_send_channel_t = lib::Channel<std::pair<ClientId, order::ClientOutboundMessages>>;
    using server_send_channel_t = lib::Channel<order::ServerOutboundMessages>;

    //@note: we use shared_ptr so we can hand these out to clients and not worry about
    // pointer invalidation
    std::shared_ptr<client_send_channel_t> incomingChannel = std::make_shared<client_send_channel_t>(); 
    FlatMap<ClientId, std::shared_ptr<server_send_channel_t>> clientChannels;
    ClientId nextClientId = 0;

    asio::awaitable<void> handleClientMessages();
public:
    /* Not thread safe: only to be executed by the MatchingEngine executor! */
    [[nodiscard]] EnginePort connect();
};

}