#pragma once
#include "ExchangeConfig.hpp"
#include "ClientConnection.hpp"
#include "MatchingEngine.hpp"

#include <asio.hpp>
#include <asio/experimental/coro.hpp>

namespace exch {

asio::experimental::generator<ClientConnection> Acceptor(
    MatchingEngine &matcher,
    ExchangeConfig &cfg,
    asio::io_context &ctx
);


}