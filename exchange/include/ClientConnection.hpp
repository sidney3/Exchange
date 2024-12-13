#include "asio/io_context.hpp"
#include <asio.hpp>
#include <MRMWChannel.hpp>
#include <SRSWChannel.hpp>
#include <OrderTypes.hpp>

namespace exch {

class ClientConnection
{
public:
    ClientConnection(
        MRMWChannel<order::ClientOutboundMessages> *clientSendChannel,
        SRSWChannel<order::ServerOutboundMessages> *serverSendChannel,
        asio::ip::tcp::socket conn,
        std::shared_ptr<asio::io_context> ctx
    );
    ~ClientConnection();
    void run();
    void stop();
private:
    asio::awaitable<void> handleServerMsg();
    asio::awaitable<void> handleClientMsg();
};

}