#pragma once
#include "EnginePort.hpp"
#include <asio.hpp>
#include <Channel.hpp>
#include <OrderTypes.hpp>

namespace exch {

class ClientConnection
{
public:
    explicit ClientConnection(
        EnginePort enginePort,
        asio::ip::tcp::socket sock);

    [[nodiscard]] asio::awaitable<void> run();
private:
    EnginePort enginePort;
    asio::ip::tcp::socket sock;

    asio::awaitable<void> handleServerMsg();
    asio::awaitable<void> handleClientMsg();
public:
    ClientConnection(ClientConnection &&) = default;
    ClientConnection &operator=(ClientConnection &&) = default;
    ClientConnection(const ClientConnection &) = delete;
    ClientConnection &operator=(const ClientConnection &) = delete;
};

}