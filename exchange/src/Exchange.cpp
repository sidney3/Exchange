#include "Exchange.hpp"
#include "ClientConnection.hpp"
#include "asio/any_io_executor.hpp"

namespace exch {

Exchange::Exchange(ExchangeConfig cfg, std::shared_ptr<asio::io_context> context)
    : cfg{cfg}
    , matchingEngine{std::make_shared<MatchingEngine>(cfg, *context)}
    , acceptor{*context}
    , context{context}
{}

asio::awaitable<void> Exchange::runMatchingEngine()
{
    co_await matchingEngine->run();
}

asio::awaitable<void> Exchange::acceptClients(asio::any_io_executor clientExecutor)
{
    asio::ip::tcp::endpoint listenEndpoint{asio::ip::tcp::v4(), cfg.listenPort};
    acceptor.open(listenEndpoint.protocol());
    acceptor.bind(listenEndpoint);
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.listen();

    fmt::println("Accepting clients");

    while(true)
    {
        asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
        EnginePort clientPort = co_await matchingEngine->connect(*context);
        fmt::println("Received client, assigned id: {}", clientPort.myId());
        ClientConnection conn{std::move(clientPort),
            std::move(socket)
        };
        clients.emplace_back(std::make_shared<ClientConnection>(std::move(conn)));
        asio::co_spawn(clientExecutor, clients.back()->run(), asio::detached);
    }
}

}