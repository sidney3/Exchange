#pragma once
#include "ClientConnection.hpp"
#include "asio/any_io_executor.hpp"
#include "asio/io_context.hpp"
#include <asio.hpp>
#include <ExchangeConfig.hpp>
#include <MatchingEngine.hpp>

namespace exch {

class Exchange
{
public:
    Exchange(ExchangeConfig cfg, std::shared_ptr<asio::io_context> context);
    asio::awaitable<void> runMatchingEngine();
    asio::awaitable<void> acceptClients(asio::any_io_executor clientExecutor);
private:
    /*
        Use pointers so that we can be moveable 
    */
    ExchangeConfig cfg;
    std::shared_ptr<MatchingEngine> matchingEngine;
    asio::ip::tcp::acceptor acceptor;
    std::vector<std::shared_ptr<ClientConnection>> clients;
    std::shared_ptr<asio::io_context> context;
public:
    Exchange(Exchange&&) = default;
    Exchange(const Exchange&) = delete;
    Exchange& operator=(Exchange&&) = default;
    Exchange& operator=(const Exchange&) = delete;
};

}