#pragma once
#include <asio.hpp>
#include <Channel.hpp>
#include <OrderTypes.hpp>
#include <Types.hpp>

namespace exch {

/*
    the interface with the Matching Engine. One of these gets handed
    to each client and they use it to send/receive messages
*/
class EnginePort 
{
public:
    explicit EnginePort(
        ClientId id
        , std::shared_ptr<lib::Channel<std::pair<ClientId, order::ClientOutboundMessages>>> toServerChan
        , std::shared_ptr<lib::Channel<order::ServerOutboundMessages>> fromServerChan
    );

    asio::awaitable<order::ServerOutboundMessages> receive();
    void send(order::ClientOutboundMessages);
private:
    ClientId clientId;
    std::reference_wrapper<lib::Channel<std::pair<ClientId, order::ClientOutboundMessages>>> clientSendChannel;
    std::reference_wrapper<lib::Channel<order::ServerOutboundMessages>> serverSendChannel;
};

} // namespace exch