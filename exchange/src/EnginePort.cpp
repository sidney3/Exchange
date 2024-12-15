#include "EnginePort.hpp"

namespace exch {

asio::awaitable<order::ServerOutboundMessages> EnginePort::receive() {
    co_return co_await serverSendChannel->async_receive(asio::use_awaitable);
}

asio::awaitable<void> EnginePort::send(order::ClientOutboundMessages msg) {
    co_await clientSendChannel->async_send(asio::error_code{}, {clientId, std::move(msg)}, asio::use_awaitable);
}

ClientId EnginePort::myId() const
{
    return clientId;
}

}