#pragma once
#include "ExchangeConfig.hpp"
#include "ClientConnection.hpp"
#include "MatchingEngine.hpp"
#include "EnginePort.hpp"
#include "asio/execution_context.hpp"

#include <asio.hpp>
#include <asio/experimental/coro.hpp>

namespace exch {

class Acceptor
{
public:
    Acceptor(ExchangeConfig cfg, std::function<asio::awaitable<EnginePort>(asio::io_context&)> nextPort);
    asio::awaitable<ClientConnection> accept(asio::io_context &);
private:
    std::function<asio::awaitable<EnginePort>(asio::io_context&)> nextPort;
    ExchangeConfig cfg;
};


}