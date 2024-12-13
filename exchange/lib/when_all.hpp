#pragma once
#include <asio.hpp>
#include <Signal.hpp>

namespace exch::lib {

template<typename ... AWAITABLE>
asio::awaitable<void> when_all(AWAITABLE &&... awaitables)
{
    auto executor = co_await asio::this_coro::executor;

    std::atomic<size_t> tasksRemaining{sizeof...(AWAITABLE)};
    Signal sig{executor};

    auto task = [&](auto &&awaitable) -> asio::awaitable<void> {
        co_await std::forward<decltype(awaitable)>(awaitable);
        tasksRemaining.fetch_sub(1);

        if(tasksRemaining.load() == 0)
        {
            sig.set();
        }
    };

    auto startTask = [&](auto &&awaitable) {
        asio::co_spawn(executor, task(std::forward<decltype(awaitable)>(awaitable)), asio::detached);
    };

    (startTask(std::forward<AWAITABLE>(awaitables)),...);

    co_await sig.wait();
}

}