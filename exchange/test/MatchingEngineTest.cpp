#include "EnginePort.hpp"
#include "OrderTypes.hpp"
#include <sync_wait.hpp>
#include <gtest/gtest.h>
#include <asio.hpp>
#include <MatchingEngine.hpp>
#include <thread>

namespace exch { 

class MatchingEngineTest : public ::testing::Test
{
protected:
    MatchingEngineTest() {
        asio::co_spawn(context, matchingEngine.run(), asio::detached);
    }

    /*
        For testing related coroutines that we would like to spawn and synchronously wait for 
    */
    asio::io_context testingContext{};
    std::jthread testingThread{[&](){
        testingContext.run();
    }};


    asio::io_context context{};
    std::jthread executorThread{[&](){
        context.run();
    }};
    ExchangeConfig testConfig{};
    MatchingEngine matchingEngine{testConfig};

    template<typename T, typename OptionalVariant>
    T assertAndGet(OptionalVariant &&v)
    {
        assert(v.has_value());
        assert(std::holds_alternative<T>(*v));
        return std::get<T>(*v);
    }
};

/*
    Client 1 sells AAPL at 6, client 2 buys AAPL at 8
*/
TEST_F(MatchingEngineTest, SimpleTrade)
{
    EnginePort client1Port = matchingEngine.connect();
    EnginePort client2Port = matchingEngine.connect();

    order::Id client1CID = 5, client2CID = 8;
    order::Qty client1Qty = 9, client2Qty = 5;
    
    // the buy is the aggressor and so it should be chosen
    // as the exec price
    order::Price sellPrice = 6, buyPrice = 8;

    order::Order AAPLSell{
        .price = sellPrice,
        .qty = client1Qty,
        .side = order::Side::Sell,
        .clientId = client1CID
    };
    order::Order AAPLBuy{
        .price = buyPrice,
        .qty = client2Qty,
        .side = order::Side::Buy,
        .clientId = client2CID
    };

    client1Port.send(AAPLSell);

    /*
        Client1 and client2 send their orders, both get acked 
    */
    auto client1Ack = assertAndGet<order::Ack>(lib::sync_wait(client1Port.receive()));
    EXPECT_EQ(client1Ack.clientId, client1CID);

    client2Port.send(AAPLBuy);
    auto client2Ack  = assertAndGet<order::Ack>(lib::sync_wait(client2Port.receive()));
    EXPECT_EQ(client2Ack.clientId, client2CID);

    auto client1Fill = assertAndGet<order::Fill>(lib::sync_wait(client1Port.receive()));
    auto client2Fill = assertAndGet<order::Fill>(lib::sync_wait(client1Port.receive()));

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