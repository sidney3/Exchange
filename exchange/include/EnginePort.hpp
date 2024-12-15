#pragma once
#include <asio.hpp>
#include <OrderTypes.hpp>
#include <Types.hpp>

namespace exch {

/*
    the interface with the Matching Engine. One of these gets handed
    to each client and they use it to send/receive messages
*/
class EnginePort 
{
private:
    ClientId clientId;
    using client_send_channel_t = channel<std::pair<ClientId, order::ClientOutboundMessages>>;
    using server_send_channel_t = channel<order::ServerOutboundMessages>;

    std::shared_ptr<client_send_channel_t> clientSendChannel;
    std::shared_ptr<server_send_channel_t> serverSendChannel;
public:
    explicit EnginePort(
        ClientId id
        , std::shared_ptr<client_send_channel_t> toServerChan
        , std::shared_ptr<server_send_channel_t> fromServerChan
    ) : clientId{id}
    , clientSendChannel{std::move(toServerChan)}
    , serverSendChannel{std::move(fromServerChan)}
    {}
    explicit EnginePort() = default;

    asio::awaitable<order::ServerOutboundMessages> receive();
    asio::awaitable<void> send(order::ClientOutboundMessages);
    ClientId myId() const;
};

} // namespace exch