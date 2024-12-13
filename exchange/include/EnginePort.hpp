#pragma once
#include <Channel.hpp>
#include <OrderTypes.hpp>

namespace exch {

/*
    the interface with the Matching Engine. One of these gets handed
    to each client and they use it to send/receive messages
*/
struct EnginePort 
{
    lib::Channel<order::ClientOutboundMessages> *clientSendChannel;
    lib::Channel<order::ServerOutboundMessages> *serverSendChannel;
};

} // namespace exch