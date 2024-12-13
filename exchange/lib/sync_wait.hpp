#pragma once
#include <asio.hpp>
#include <optional>

namespace exch::lib {

template<typename AWAITABLE, typename ReturnT = typename AWAITABLE::value_type>
std::optional<ReturnT> sync_wait(AWAITABLE &&awaitable)
{
    asio::io_context tempContext;

    std::thread th{[&](){
        tempContext.run();
    }};

    return asio::co_spawn(tempContext, std::forward<AWAITABLE>(awaitable), asio::use_future);
}
} // namespace exch::lib