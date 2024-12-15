#pragma once
#include "ClientConnection.hpp"
#include "asio/any_io_executor.hpp"
#include "asio/io_context.hpp"
#include <asio.hpp>
#include <ExchangeConfig.hpp>
#include <MatchingEngine.hpp>
#include <Acceptor.hpp>

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
    std::shared_ptr<MatchingEngine> matchingEngine;
    std::shared_ptr<Acceptor> acceptor;
    std::vector<std::shared_ptr<ClientConnection>> clients;

    std::shared_ptr<asio::io_context> context;
};

}