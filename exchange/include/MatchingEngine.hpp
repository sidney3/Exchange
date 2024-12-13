#pragma once
#include "ExchangeConfig.hpp"
#include <asio.hpp>
#include <MRMWChannel.hpp>
#include <SRSWChannel.hpp>
#include <OrderTypes.hpp>

namespace exch {

class MatchingEngine
{
public:
    MatchingEngine(ExchangeConfig cfg);
    asio::awaitable<void> run();
public:
    /*
        Public interface with the Matching Engine 
    */
    struct ConnectPort
    {
        lib::MRMWChannel<order::ClientOutboundMessages> *clientSendChannel;
        lib::SRSWChannel<order::ServerOutboundMessages> *serverSendChannel;
    };

    asio::awaitable<ConnectPort> openPort();
private:
    asio::awaitable<void> handleClientMessage();
};

}