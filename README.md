# Exchange

My final project for Networks: a mock exchange

## Architecture Flow

This is a coroutine based system, where each module is defined in terms of a class that holds its state, and a `asio::awaitable<void> run()` member function of that class. These can be composed using `template<typename ... AWAITABLES> when_all(AWAITABLES&&...)`.

An example of this style of module, and an example driver for it, is given in `exchange/include/TestAsyncApplication.hpp` and `exchange/src/TestDriver.cpp` respectively.

## Objectives

## Non-Objectives

As we are more interested in exploring building something efficient, we eschew:

1. Cancels
2. Handling

## Design Choices