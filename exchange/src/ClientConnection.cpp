#include "ClientConnection.hpp"
#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include <when_all.hpp>
#include <FIX.hpp>
#include <fmt/format.h>

namespace exch {

ClientConnection::ClientConnection(EnginePort enginePort, asio::ip::tcp::socket sock)
    : enginePort{enginePort}
    , sock{std::move(sock)}
    , sendBuffer(FIX::MaxMessageSize, '\0')
{}

asio::awaitable<void> ClientConnection::run()
{
    co_await lib::when_all(
        handleClientMsg(),
        handleServerMsg()
    );
}

struct FixEncoder
{
    static size_t encodeAck(order::Ack &ack, std::vector<char> &buf) {
        return -1;
    }
    static size_t encodeFill(order::Fill &fill, std::vector<char> &buf) {
        return -1;
    }
    static std::optional<order::Order> decodeOrder(FIX::Message &order) {
        return std::nullopt;
    }
};

asio::awaitable<void> ClientConnection::handleServerMsg()
{
    while(true)
    {
        order::ServerOutboundMessages msg = co_await enginePort.receive();

        size_t messageSize = std::visit(Overloaded{
            [&](order::Ack &ack)
            {
                return FixEncoder::encodeAck(ack, sendBuffer);
            },
            [&](order::Fill &fill)
            {
                return FixEncoder::encodeFill(fill, sendBuffer);
            }
        }, msg);

        co_await sock.async_send(asio::buffer(sendBuffer.data(), messageSize), asio::use_awaitable);
    }
}

asio::awaitable<void> ClientConnection::handleClientMsg()
{
    while(true)
    {
        std::optional<FIX::Message> maybeFIXOrder = co_await FIX::readOrder(sock);
        if(!maybeFIXOrder.has_value())
        {
            fmt::println("Received malformatted order");
            continue;
        }

        std::optional<order::Order> maybeOrder = FixEncoder::decodeOrder(*maybeFIXOrder);
        if(!maybeOrder.has_value())
        {
            fmt::println("Received valid FIX message, but not valid order");
            continue;
        }
        co_await enginePort.send(std::move(*maybeOrder));
    }
}

}
