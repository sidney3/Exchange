# Exchange

My final project for Networks: a mock exchange

## Architecture Flow

This is a coroutine based system, where each module is defined in terms of a class that holds its state, and a `asio::awaitable<void> run()` member function of that class. These can be composed using `template<typename ... AWAITABLES> when_all(AWAITABLES&&...)`.

An example of this style of module, and an example driver for it, is given in `exchange/include/TestAsyncApplication.hpp` and `exchange/src/TestDriver.cpp` respectively.


## Objectives

1. Try out building an asyncronous system. I felt I wrote a very inefficient thread based TCP system that spent a lot of its time spinning, and I wanted to try out building a system around coroutines

2. Make something performant (or at least investigate performance). I put special emphasis on making something that was sufficiently modular to write benchmarks for it. We spend most of the report discussing our timing related findings.

## Non-Objectives

As we are more interested in exploring building something efficient, we eschew:

1. Cancels
2. Network card performance. At a high level, we can the flow of data in our program is as follows:

Incoming Data (over the wire) in the FIX format  --->

`ClientConnection.hpp` (translated into our internal order structs)  --->

`MatchingEngine.hpp` (feeds the orders into a single threaded `OrderBook.hpp`)

We propose implementations for that first link, but don't test these thoroughly and instead focus our testing + benchmarking time on the communication between `ClientConnection` and `MatchingEngine`.

There is a lot of depth to the first step (how do we do our encodings / decodings in a performant way), but I wasn't able to find the time to focus on this link of the graph. So we spend most of our time thinking about the link between the client connection and the matching engine. The `exchange/lib/Fix.hpp` are the main areas where I would have liked to put more time into thinking about performance, but this is a challenge for another day.

## Performance Writeup

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

### How many threads to use?

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

```
-------------------------------------------------------------------------------------
Benchmark Of Single Threaded Channel                Time             CPU   Iterations
-------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000     94089550 ns        25028 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000   450907438 ns       248491 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/250/1/1000 4717823168 ns      3196460 ns            1 Num clients: 250, Num worker threads: 1, Trades Per Client: 1000

--------------------------------------------------------------------------------------
Benchmark Of Multithreaded Channel                   Time             CPU   Iterations
--------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000     102084023 ns        29470 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000    458553909 ns       249258 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/250/1/1000  4625691211 ns      3100376 ns            1 Num clients: 250, Num worker threads: 1, Trades Per Client: 1000
```

We will happily take a 5% performance gain (and a more modest ~1% gain in the 25 client case). As we only get 1 sample with the 250 client case I do not consider this statistically significant

### CoAwait the executions?

We do not want to send the client an `Ack` for an order after we send them a `Fill` for that order, and so our engine is forced to `co_await` it's `Ack` to the client (i.e. it can't dry fire it). But can we get any performance gains from dry-firing our `Executions`? Note that we do this in an environment with exactly 1 client, else we can get some deadlocks (not due to an error in the matching engine, but due to the somewhat naive way in which we do our benchmarks). 

```
-----------------------------------------------------------------------------------
Benchmark Of Dry Firing Executions                Time             CPU   Iterations
-----------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/1/1/1000  102749174 ns         8577 ns          100 Num clients: 1, Num worker threads: 1, Trades Per Client: 1000
-----------------------------------------------------------------------------------
Benchmark Of Awaiting Client Executions          Time             CPU   Iterations
-----------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/1/1/1000  102958890 ns         9564 ns          100 Num clients: 1, Num worker threads: 1, Trades Per Client: 1000
```

And interestingly, we find very little performance gain from dry-firing our executions. I was very surprised by this, and it speaks to the efficiency of `asio`'s async runtime.

### Use better data structures

Not a very surprising part of our benchmarks, but we evaluate the performance advantages that we gain from switching to `StackVector` (vector that starts out stack allocated past a threshold) and `FlatMap` (open addressed hash map) in place of `std::vector` and `std::unordered_map`. We use the sick (abseil)[https://abseil.io/] library 

```
------------------------------------------------------------------------------------
Initial Benchmark                                          Time             CPU   Iterations
------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000    95265023 ns        30415 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000  453356829 ns       237687 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000
------------------------------------------------------------------------------------
Linear Probing Hashmap Benchmark                                          Time             CPU   Iterations
------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000    95019727 ns        27491 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000  471808450 ns       254275 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000

------------------------------------------------------------------------------------
Linear Probing + Stack Vector Time             CPU   Iterations
------------------------------------------------------------------------------------
BM_MatchingEngineSendSomeTrades/5/1/1000    96821576 ns        27863 ns          100 Num clients: 5, Num worker threads: 1, Trades Per Client: 1000
BM_MatchingEngineSendSomeTrades/25/1/1000  467132430 ns       247230 ns           10 Num clients: 25, Num worker threads: 1, Trades Per Client: 1000

```

Most surprising in the above is how little benefit we get from the use of stack vectors (note that our `OrderBook` returns one of these for every execution). I would imagine that not allocating would give us a lot of performance, but surprisingly it did not, so we remain with `std::vector`.
