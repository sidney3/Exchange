#pragma once
#include "ExchangeConfig.hpp"
#include "ClientConnection.hpp"
#include "MatchingEngine.hpp"
#include "EnginePort.hpp"

#include <asio.hpp>
#include <asio/experimental/coro.hpp>

namespace exch {

class Acceptor
{
public:
    Acceptor(ExchangeConfig cfg, 
        asio::experimental::generator<EnginePort> nextPort
    );
    asio::awaitable<ClientConnection> accept();
private:
    asio::experimental::generator<EnginePort> nextPort;
    ExchangeConfig cfg;
};


}