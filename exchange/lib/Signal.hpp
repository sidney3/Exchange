#pragma once
#include <asio.hpp>
#include <chrono>

namespace exch::lib {

class Signal
{
private:
    asio::steady_timer timer;
public:
    explicit Signal(asio::any_io_executor exec)
        : timer{exec}
    {}

    asio::awaitable<void> wait() {
        timer.expires_at(std::chrono::steady_clock::time_point::max());
        co_await timer.async_wait(asio::use_awaitable);
    }
    void set() {
        timer.cancel();
    }
};
}