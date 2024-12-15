#include "Acceptor.hpp"

namespace exch {


Acceptor::Acceptor(ExchangeConfig cfg, std::function<asio::awaitable<EnginePort>(asio::io_context&)> nextPort) 
    : nextPort{std::move(nextPort)}
    ,cfg{cfg}

{}


asio::awaitable<ClientConnection> Acceptor::accept(asio::io_context &context) {
    throw std::runtime_error{"Not implemented"};
}

}