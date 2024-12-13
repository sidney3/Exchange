#pragma once
#include <asio.hpp>
#include <chrono>
#include <when_all.hpp>
#include <fmt/format.h>

/*
    An example coroutine based class to show our desired flow
    
    (this gets run in exchange/src/TestDriver)
*/
class AsyncApplication
{
public:
    asio::awaitable<void> run() {
        co_await exch::lib::when_all(everyOneSecond(), everyTwoSeconds());
    }
    void stop() {
        running = false;
    }
private:
    bool running = true;
    asio::awaitable<void> everyOneSecond() {
        auto executor = co_await asio::this_coro::executor;

        asio::steady_timer timer{executor};

        while(running)
        {
            timer.expires_after(std::chrono::seconds(1));
            co_await timer.async_wait(asio::use_awaitable);
            fmt::println("1 second");
        }
    }
    asio::awaitable<void> everyTwoSeconds() {
        auto executor = co_await asio::this_coro::executor;

        asio::steady_timer timer{executor};

        while(running)
        {
            timer.expires_after(std::chrono::seconds(2));
            co_await timer.async_wait(asio::use_awaitable);
            fmt::println("2 seconds");
        }
    }
};