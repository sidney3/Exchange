#include "OrderTypes.hpp"
#include "Types.hpp"
#include <EnginePort.hpp>
#include <MatchingEngine.hpp>
#include <asio.hpp>
#include <benchmark/benchmark.h>
#include <thread>
#include <random>
#include <variant>

static void BM_MatchingEngineSendSomeTrades(benchmark::State &state) {
    asio::io_context context;
    auto workGuard = asio::make_work_guard(context.get_executor());
    std::jthread engineWorker{[&](){
        context.run();
    }};
    exch::ExchangeConfig cfg{
        .listenPort = 1010
    };
    exch::MatchingEngine matchingEngine{cfg, context};
    asio::co_spawn(context, matchingEngine.run(), asio::detached);

    size_t numClients = state.range(0);
    size_t numTradesPerClient = state.range(1);


    auto randomOrder = [](
        exch::ClientId id, 
        std::mt19937 &generator, 
        std::uniform_real_distribution<exch::order::Price> &priceDistribution, 
        std::uniform_int_distribution<exch::order::Qty> &qtyDistribution
    )
    {
        auto randomSide= [&](){
            return (qtyDistribution(generator) & 1) ?
                exch::order::Side::Buy :
                exch::order::Side::Sell;
        };
        return exch::order::Order{
            .price = priceDistribution(generator),
            .qty = qtyDistribution(generator),
            .side = randomSide(),
            .symbol = "AAPL",
            .sendTime = std::chrono::steady_clock::now(),
            .foreignOrderId = 0,
            .clientId = id
        };
    };
    auto clientThread = [&](exch::EnginePort &connection) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<exch::order::Price> priceDistribution(0.50, 100);
        std::uniform_int_distribution<exch::order::Qty> qtyDistribution(5, 100);

        for(size_t i = 0; i < numTradesPerClient; ++i) {
            asio::co_spawn(context, connection.send(randomOrder(connection.myId(), gen, priceDistribution, qtyDistribution)), asio::detached);
            
            while(!std::holds_alternative<exch::order::Ack>(asio::co_spawn(context, connection.receive(), asio::use_future).get())) {
                // spin until we get the ack for our order back
                // we might get executions but we only care about getting our order acked
            }
        }
    };

    std::vector<exch::EnginePort> clientConnections;
    for(size_t i = 0; i < numClients; ++i)
    {
        exch::EnginePort clientPort = asio::co_spawn(context, matchingEngine.connect(context), asio::use_future).get();
        clientConnections.emplace_back(std::move(clientPort));
    }

    for(auto _ : state)
    {
        state.PauseTiming();
        std::vector<std::jthread> clientThreads;
        for(exch::EnginePort &port : clientConnections)
        {
            clientThreads.emplace_back(clientThread, std::ref(port));
        }
        state.ResumeTiming();

        for(std::jthread &jt : clientThreads)
        {
            jt.join();
        }
    }

    context.stop();
}

BENCHMARK(BM_MatchingEngineSendSomeTrades)
    ->Args({1, 100});

BENCHMARK_MAIN();