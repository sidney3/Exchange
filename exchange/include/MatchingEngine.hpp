#pragma once
#include "ExchangeConfig.hpp"
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
public:
    [[nodiscard]] asio::experimental::generator<EnginePort> connect();
public:
    MatchingEngine(MatchingEngine &&) = default;
    MatchingEngine(const MatchingEngine &) = delete;
    MatchingEngine &operator=(MatchingEngine &&) = default;
    MatchingEngine &operator=(const MatchingEngine &) = delete;
private:
    OrderBook book;
    asio::awaitable<void> handleClientMessages();
};

}