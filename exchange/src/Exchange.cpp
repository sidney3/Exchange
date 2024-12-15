#include "Exchange.hpp"
#include "ClientConnection.hpp"
#include "asio/any_io_executor.hpp"

namespace exch {

Exchange::Exchange(ExchangeConfig cfg, std::shared_ptr<asio::io_context> context)
    : matchingEngine{std::make_shared<MatchingEngine>(cfg, *context)}
    , acceptor{std::make_shared<Acceptor>(cfg, *context, [me = this->matchingEngine](asio::io_context &ctx) {
        return me->connect(ctx);
    })}
    , clients{}
    , context{context}
{}

asio::awaitable<void> Exchange::runMatchingEngine()
{
    co_await matchingEngine->run();
}

asio::awaitable<void> Exchange::acceptClients(asio::any_io_executor clientExecutor)
{
    while(1)
    {
        ClientConnection conn = co_await acceptor->accept(*context);
        clients.emplace_back(std::make_shared<ClientConnection>(std::move(conn)));
        asio::co_spawn(clientExecutor, clients.back()->run(), asio::detached);
    }
}

}