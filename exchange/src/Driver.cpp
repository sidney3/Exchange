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
    asio::io_context engineExecutor;
    asio::io_context clientExecutor;

    std::thread engineWorker{[&](std::stop_token tok){
        engineExecutor.run();
    }};
    std::thread clientWorker{[&](std::stop_token tok){
        clientExecutor.run();
    }};

    exch::ExchangeConfig cfg{
        .listenPort = 1010
    };
    
    /*
        Core of the exchange 
    */
    exch::MatchingEngine matchingEngine{cfg};
    
    /*
        Co-routine that spends its life accepting clients and transferring them
        to the clientExecutor for work 
    */
    auto clientAcceptor = [&]() -> asio::awaitable<void> {
        exch::Acceptor acceptor{cfg, matchingEngine.connect()};
        std::vector<exch::ClientConnection> activeClients;

        while(true)
        {
            activeClients.emplace_back(co_await acceptor.accept());
            asio::co_spawn(clientExecutor, activeClients.back().run(), asio::detached);
        }
    };

    asio::co_spawn(engineExecutor, matchingEngine.run(), asio::detached);
    asio::co_spawn(engineExecutor, clientAcceptor(), asio::detached);

    while(1);
}