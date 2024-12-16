#include "Types.hpp"
#include <Exchange.hpp>

#include <iostream>
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{
    if(argc != 2)
    {
        throw std::runtime_error{"Usage: ./driver <listen-port>"};
    }

    std::shared_ptr<asio::io_context> context = std::make_shared<asio::io_context>();
    auto workGuard = asio::make_work_guard(context->get_executor());

    std::jthread engineWorker{[&](std::stop_token tok){
        context->run();
    }};

    exch::PortNum listenPortNum = std::stoi(argv[1]);
    exch::ExchangeConfig cfg{
        .listenPort = listenPortNum
    };
    
    exch::Exchange exchange{cfg, context};

    asio::co_spawn(context->get_executor(), exchange.acceptClients(context->get_executor()), asio::detached);
    asio::co_spawn(context->get_executor(), exchange.runMatchingEngine(), asio::detached);

    constexpr char stopCmd = 'S';
    std::string userInput;
    while(true)
    {
        std::cout << ">";
        std::getline(std::cin, userInput);
        if(userInput.empty())
        {
            continue;
        }
        switch(userInput.front())
        {
        case stopCmd: {
            context->stop();
            return 0;
        }
        default: {
            break;
        }
        }
    }
}