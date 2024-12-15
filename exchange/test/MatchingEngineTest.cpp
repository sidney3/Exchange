#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include "asio/executor_work_guard.hpp"
#include "asio/io_context.hpp"
#include <gtest/gtest.h>
#include <asio.hpp>
#include <MatchingEngine.hpp>
#include <fmt/format.h>
#include <thread>

namespace exch { 

using namespace std::chrono;

class MatchingEngineTest : public ::testing::Test
{
protected:
    MatchingEngineTest() :
        workGuard{context.get_executor()}
        , workerThread{[&](){
            context.run();
        }}
    {
        std::cout << "MatchingEngineTest(): construct\n";
        asio::co_spawn(context, matchingEngine.run(), asio::detached);
    }
    ~MatchingEngineTest()
    {
        workGuard.reset();
        context.stop();
    }

    /*
        For testing related coroutines that we would like to spawn and synchronously wait for 
    */
    asio::io_context context{};
    asio::executor_work_guard<asio::io_context::executor_type> workGuard;
    std::jthread workerThread;


    ExchangeConfig testConfig{};
    MatchingEngine matchingEngine{testConfig, context};

    template<typename AWAITABLE, typename ReturnT = typename std::decay_t<AWAITABLE>::value_type>
    ReturnT sync_wait(AWAITABLE &&awaitable, std::chrono::steady_clock::duration waitTime = 50ms)
    {
        std::future<ReturnT> fut = asio::co_spawn(context, std::forward<AWAITABLE>(awaitable), asio::use_future);
        if(fut.wait_for(waitTime) != std::future_status::ready)
        {
            throw std::runtime_error{fmt::format("Sync wait did not complete within: {}ms", duration_cast<milliseconds>(waitTime).count())};
        }
        return fut.get();
    }
    EnginePort connect() {
        return sync_wait(matchingEngine.connect(context));
    }
    template<typename T, typename OptionalVariant>
    T assertAndGet(OptionalVariant &&v)
    {
        assert(std::holds_alternative<T>(v));
        return std::get<T>(v);
    }
};

/*
    Client 1 sells AAPL at 6, client 2 buys AAPL at 8
*/
TEST_F(MatchingEngineTest, SimpleTrade)
{
    EnginePort client1Port = connect();
    EnginePort client2Port = connect();

    order::Id client1OID = 0, client2OID = 1;
    order::Qty client1Qty = 9, client2Qty = 5;
    
    // the buy is the aggressor and so it should be chosen
    // as the exec price
    order::Price sellPrice = 6, buyPrice = 8;
    std::string SYM = "AAPL";

    order::Order AAPLSell{
        .price = sellPrice,
        .qty = client1Qty,
        .side = order::Side::Sell,
        .symbol = SYM,
        .foreignOrderId = client1OID,
        .clientId = client1Port.myId()
    };
    order::Order AAPLBuy{
        .price = buyPrice,
        .qty = client2Qty,
        .side = order::Side::Buy,
        .symbol = SYM,
        .foreignOrderId = client2OID,
        .clientId = client2Port.myId()
    };

    sync_wait(client1Port.send(AAPLSell));

    /*
        Client1 and client2 send their orders, both get acked 
    */
    auto client1Ack = assertAndGet<order::Ack>(sync_wait(client1Port.receive()));
    EXPECT_EQ(client1Ack.clientId, client1OID);

    sync_wait(client2Port.send(AAPLBuy));
    auto client2Ack  = assertAndGet<order::Ack>(sync_wait(client2Port.receive()));
    EXPECT_EQ(client2Ack.clientId, client2OID);

    auto client1Fill = assertAndGet<order::Fill>(sync_wait(client1Port.receive()));
    auto client2Fill = assertAndGet<order::Fill>(sync_wait(client2Port.receive()));

    EXPECT_EQ(client1Fill.id, client1Ack.clientId);
    EXPECT_EQ(client2Fill.id, client2Ack.clientId);
    
    order::Qty expectedQty = std::min(client1Qty, client2Qty);
    order::Price expectedPrice = buyPrice;

    EXPECT_EQ(client1Fill.fillQty, expectedQty);
    EXPECT_EQ(client2Fill.fillQty, expectedQty);
    EXPECT_EQ(client1Fill.fillPrice, expectedPrice);
    EXPECT_EQ(client2Fill.fillPrice, expectedPrice);
}

}