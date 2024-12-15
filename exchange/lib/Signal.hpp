#pragma once
#include <asio.hpp>
#include <chrono>
#include <mutex>
#include <optional>

namespace exch::lib {

class Signal
{
private:
    std::mutex m;
    std::optional<asio::steady_timer> timer;
public:
    explicit Signal(): timer{}
    {}

    asio::awaitable<void> wait() {
        auto executor = co_await asio::this_coro::executor;
        {
            std::lock_guard lk(m);
            timer = asio::steady_timer{executor};
            timer->expires_at(std::chrono::steady_clock::time_point::max());
        }
        co_await timer->async_wait(asio::use_awaitable);
    }
    void set() {
        std::lock_guard lk{m};
        if(timer.has_value())
            timer->cancel();
    }
};
}