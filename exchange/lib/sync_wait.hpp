#pragma once
#include <asio.hpp>

namespace exch::lib {

/*
    Synchronously wait for the result of an AWAITABLE computation
*/
template<typename AWAITABLE>
typename AWAITABLE::value_type sync_wait(AWAITABLE &&awaitable) {
    asio::io_context tempContext;

    asio::co_spawn(tempContext, std::forward<AWAITABLE>(awaitable), [&]() {

    });
}


}