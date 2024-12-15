#include "Acceptor.hpp"
#include "ClientConnection.hpp"
#include <Fix.hpp>
#include <fmt/format.h>

namespace exch {


Acceptor::Acceptor(ExchangeConfig cfg, asio::io_context &context, std::function<asio::awaitable<EnginePort>(asio::io_context&)> nextPort) 
    : nextPort{std::move(nextPort)}
    , acceptor{context}
    , cfg{cfg}
{
    asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), cfg.listenPort);
    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor.bind(endpoint);
    acceptor.listen();
}


asio::awaitable<ClientConnection> Acceptor::accept(asio::io_context &context) {
    asio::ip::tcp::socket socket = co_await acceptor.async_accept(asio::use_awaitable);
    std::optional<FIX::Message> maybeLogon = co_await FIX::readLogon(socket);

    if(maybeLogon.has_value())
    {
        co_return ClientConnection{
            co_await nextPort(context),
            std::move(socket)
        };
    }
    else
    {
        fmt::println("Bad logon for client from address: {}", socket.remote_endpoint().address().to_string());
    }
}

}