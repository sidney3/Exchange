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

## Writeup

As mentioned above, one of the big goals of this project was to both build a functional product, but then benchmark it and improve accordingly. We discuss our benchmarking methods and some of our surprising findings.

### Benchmark Methodology

The core of our exchange is the `MatchingEngine`. It exposes a method `MatchingEngine::awaitable<EnginePort>` where `EnginePort` is as follows:

```cpp
class EnginePort {
    asio::awaitable<order::ServerOutboundMessages> receive();
    asio::awaitable<void> send(order::ClientOutboundMessages);
}
```

As I'm not interested in testing my network card, but instead the performance of the `MatchingEngine`, this means that the benchmarks that we run don't actually send/receive any bytes over the wire, but instead...

1. Create a matching engine and our async runtime
2. Spin up some "client" threads that send trades and wait to receive an ack for the trades (note that these are mock clients)

This can be seen in `exchange/benchmark/MatchingEngineBenchmark.cpp`. Each client sends 1000 trades and critically does not fire all trades and wait for 1000 acks, but instead fires trades sequentially, only sending the next trade after the prior has been Acked. I felt this was more reflective of how a real client might send trades.

In our benchmarks, we also allow "self-trades" (trading with yourself). This is configurable (see `OrderBook.hpp`), and in a real exchange environment would be disabled, but this led to an unecessary performance hit for situations where we had very few threads trading.

### How many threads to use

Because our architecture gets represented by a series of `asio::awaitable<void>` tasks, we can easily vary the amount of work that we would like to distribute across our system. We explore three different strategies:

1. One total thread (handles both client processing and matching engine)
2. Two total threads (one for client processing, one for matching engine)
3. N + 1 total threads (one thread per client, one for matching engine)

We test these three strategies across a variety of client loads (5, 25, and 250 clients). We note that, for strategy 3, we use `min(std::thread::hardware_concurrency(), N)` where `hardware_concurrency()` gives the number of available threads in our system. In each test, each client sends 1000 trades (we just make this number high to minimize the relative time spent on initialization, as in a real trading environment this will be done _before_ the trading day so we don't care about it)

```
--------------------------------------------------------------------------------------
Benchmark                                            Time             CPU   Iterations
--------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000     102084023 ns        29470 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/5/2/1000     131594124 ns        29968 ns          100 Num clients: 5, Num worker threads: 2, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/5/6/1000     722536646 ns        30608 ns           10 Num clients: 5, Num worker threads: 6, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000    458553909 ns       249258 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/2/1000    578258455 ns       184250 ns           10 Num clients: 25, Num worker threads: 2, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/12/1000  3697494085 ns       202542 ns            1 Num clients: 25, Num worker threads: 12, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/250/1/1000  4625691211 ns      3100376 ns            1 Num clients: 250, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/250/2/1000  5403632336 ns      1710460 ns            1 Num clients: 250, Num worker threads: 2, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/250/12/1000 3.7457e+10 ns      2165291 ns            1 Num clients: 250, Num worker threads: 12, Trades Per Client: 1000
```

Most surprisingly, the 1 thread approach is always faster than any other approach in real time. Interestingly, we see an improvement in CPU time under higher client workloads when we use more threads (this can be thought of as "our asyncronous system spending more time asleep"). I would suspect, then, that the extra time we see in the multithreaded environment is due to more pressure on the cache from many threads. Therefore, we choose to use a single threaded approach.

Note that in all of the above tests, we use `asio::concurrent_channel`, but as a result of these findings, we choose to use `asio::channel` (as we make our whole system single threaded).

We also note that the above does not include the third work source of accepting clients, but as we expect this to be extremely rare (compared to sending trades), we aren't worried about its affects on our benchmark.

### Concurrent vs Single Threaded Channel

So we choose to use a single threaded system. How much extra performance do we get from the multithreaded vs single threaded channel implementations provided by asio?
