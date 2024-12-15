#include "EnginePort.hpp"
#include "asio/any_io_executor.hpp"
#include <asio/experimental/coro.hpp>
#include <asio.hpp>
#include <Acceptor.hpp>
#include <ExchangeConfig.hpp>
#include <ClientConnection.hpp>
#include <MatchingEngine.hpp>

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    asio::io_context context;

    std::thread engineWorker{[&](std::stop_token tok){
        context.run();
    }};

    exch::ExchangeConfig cfg{
        .listenPort = 1010
    };
    
    /*
        Core of the exchange 
    */
    exch::MatchingEngine matchingEngine{cfg, context};
    

    /*
        Co-routine that spends its life accepting clients and transferring them
        to the clientExecutor for work 
    */
    auto clientAcceptor = [&]() -> asio::awaitable<void> {
        exch::Acceptor acceptor{cfg, [&](asio::io_context &ctx) {return matchingEngine.connect(ctx);}};
        std::vector<exch::ClientConnection> activeClients;

        while(true) {
            activeClients.emplace_back(co_await acceptor.accept(context));
            asio::co_spawn(context, activeClients.back().run(), asio::detached);
        }
    };

    asio::co_spawn(context, matchingEngine.run(), asio::detached);
    asio::co_spawn(context, clientAcceptor(), asio::detached);

    while(1);
}