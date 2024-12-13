#include <asio.hpp>
// #include <Acceptor.hpp>
// #include <ExchangeConfig.hpp>
// #include <ClientConnection.hpp>
// #include <MatchingEngine.hpp>
#include <TestAsyncApplication.hpp>

#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    // asio::io_context matchingEngineExecutor;
    // asio::io_context clientExecutor;

    // std::jthread matchingEngineThread{[&](std::stop_token tok){
    //     matchingEngineExecutor.run();
    // }};
    // std::jthread clientThread{[&](std::stop_token tok){
    //     // while(!tok.stop_requested())
    //     // {
    //     //     clientExecutor.run_one();
    //     // }
    //     clientExecutor.run();
    // }};

    // std::vector<exch::ClientConnection> activeConnections;
    // exch::ExchangeConfig cfg{};
    // exch::MatchingEngine me{cfg};

    asio::io_context context;
    AsyncApplication app;
    asio::co_spawn(context, app.run(), asio::detached);

    std::thread jt{[&](){
        context.run();
    }};

    std::this_thread::sleep_for(std::chrono::seconds(20));

    app.stop();
    context.stop();
    jt.join();
}