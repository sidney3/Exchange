#include "ClientConnection.hpp"
#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include <when_all.hpp>
#include <FIX.hpp>
#include <fmt/format.h>
#include <time.h>

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
        FIX::Message message;
        message[35] = "8"; // MsgType = Execution Report
        message[150] = "0"; // ExecType = New
        message[39] = "0"; // OrdStatus = New
        message[11] = ack.clientId; // Client Order ID
        message[37] = ack.vendorId; // Vendor Order ID

        return FIX::encodeAck(message, buf);
    }

    static size_t encodeFill(order::Fill &fill, std::vector<char> &buf) {
        FIX::Message message;
        message[35] = "8"; // MsgType = Execution Report
        message[150] = "2"; // ExecType = Fill
        message[39] = "2"; // OrdStatus = Filled
        message[11] = fill.id; // Client Order ID
        message[31] = std::to_string(fill.fillPrice); // LastPx (fill price)
        message[32] = std::to_string(fill.fillQty);   // LastQty (fill quantity)

        return FIX::encodeFill(message, buf);
    }

    static std::optional<order::Order> decodeOrder(FIX::Message &fixMessage) {
        try {
            order::Order order;

            if (fixMessage[35] != "D") { 
                return std::nullopt;
            }

            order.foreignOrderId = std::stoi(fixMessage[11]); 
            order.clientId = std::stoi(fixMessage[49]);      
            order.symbol = fixMessage[55];       
            order.price = std::stod(fixMessage[44]);
            order.qty = std::stod(fixMessage[38]); 

            std::string side = fixMessage[54]; // Side
            if (side == "1") {
                order.side = order::Side::Buy;
            } else if (side == "2") {
                order.side = order::Side::Sell;
            } else {
                return std::nullopt; // Invalid side
            }

            std::string type = fixMessage[40]; // OrdType
            
            // Only support limit orders!
            order.type = order::Type::Limit;

            order.sendTime = std::chrono::steady_clock::now();
            return order;
        } catch (...) {
            return std::nullopt; // Handle parsing errors
        }
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
