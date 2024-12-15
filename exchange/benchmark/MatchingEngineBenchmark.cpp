#include "OrderTypes.hpp"
#include "Types.hpp"
#include <EnginePort.hpp>
#include <MatchingEngine.hpp>
#include <asio.hpp>
#include <benchmark/benchmark.h>
#include <thread>
#include <random>
#include <variant>
#include <fmt/format.h>

static void BM_MatchingEngineSendSomeTrades(benchmark::State &state) {
    asio::io_context context;
    auto workGuard = asio::make_work_guard(context.get_executor());
    exch::ExchangeConfig cfg{
        .listenPort = 1010
    };
    exch::MatchingEngine matchingEngine{cfg, context};
    auto engineStrand = asio::make_strand(context);
    asio::co_spawn(engineStrand, matchingEngine.run(), asio::detached);

    size_t numClients = state.range(0);
    size_t numWorkerThreads = state.range(1);
    size_t numTradesPerClient = state.range(2);

    std::vector<std::jthread> workerThreads;
    for(size_t i = 0; i < numWorkerThreads; ++i)
    {
        workerThreads.emplace_back([&](){
            context.run();
        });
    }

    auto randomOrder = [](
        exch::ClientId id, 
        exch::order::Id foreignId,
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
            .foreignOrderId = foreignId,
            .clientId = id
        };
    };
    auto clientThread = [&](exch::EnginePort &connection) {
        auto clientStrand = asio::make_strand(context.get_executor());
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<exch::order::Price> priceDistribution(0.50, 100);
        std::uniform_int_distribution<exch::order::Qty> qtyDistribution(5, 100);

        exch::order::Id maxId = numTradesPerClient - 1;
        for(exch::order::Id i = 0; i <= maxId; ++i) {
            asio::co_spawn(clientStrand, connection.send(randomOrder(connection.myId(), i, gen, priceDistribution, qtyDistribution)), asio::detached);
            
            while(!std::holds_alternative<exch::order::Ack>(asio::co_spawn(clientStrand, connection.receive(), asio::use_future).get())) {
                // spin until we get the ack for our order back
                // we might get executions but we only care about getting our order acked
            }
        }
    };

    std::vector<exch::EnginePort> clientConnections;
    for(size_t i = 0; i < numClients; ++i)
    {
        exch::EnginePort clientPort = asio::co_spawn(engineStrand, matchingEngine.connect(context), asio::use_future).get();
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
    
    state.SetLabel(fmt::format("Num clients: {}, Num worker threads: {}, Trades Per Client: {}", numClients, numWorkerThreads, numTradesPerClient));
    context.stop();
}

BENCHMARK(BM_MatchingEngineSendSomeTrades)
    ->Args({5, 1, 1000})
    ->Args({5, 6, 1000});

BENCHMARK_MAIN();