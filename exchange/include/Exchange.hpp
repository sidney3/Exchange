#pragma once
#include <asio.hpp>
#include <ExchangeConfig.hpp>
#include <MatchingEngine.hpp>
#include <Acceptor.hpp>

namespace exch {

class Exchange
{
public:
    Exchange(ExchangeConfig cfg);
    asio::awaitable<void> runMatchingEngine();
    asio::awaitable<void> acceptClients();
    asio::awaitable<void> serviceClients();
private:
    MatchingEngine matchingEngine;
    Acceptor acceptor;
};

}