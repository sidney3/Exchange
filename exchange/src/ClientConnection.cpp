#include "ClientConnection.hpp"
#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include <when_all.hpp>

namespace exch {

ClientConnection::ClientConnection(EnginePort enginePort, asio::ip::tcp::socket sock)
    : enginePort{enginePort}
    , sock{std::move(sock)}
{}

asio::awaitable<void> ClientConnection::run()
{
    co_await lib::when_all(
        handleClientMsg(),
        handleServerMsg()
    );
}

asio::awaitable<void> ClientConnection::handleServerMsg()
{
    while(true)
    {
        order::ServerOutboundMessages msg = co_await enginePort.serverSendChannel->receive();
    }
}

asio::awaitable<void> ClientConnection::handleClientMsg()
{
    while(true)
    {

    }
}

}
