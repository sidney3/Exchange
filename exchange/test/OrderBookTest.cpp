#include <OrderTypes.hpp>
#include <gtest/gtest.h>
#include <OrderBook.hpp>
#include <algorithm>
#include <chrono>

namespace exch {

using namespace std::chrono;

class OrderBookTest : public ::testing::Test
{
protected:
    order::BBGTicker SYM = "AAPL";

    order::Id activeId = 0;
    order::Id nextOrderId() {
        return activeId++;
    }
    ClientId client1Id = 0, client2Id = 1;

    order::AckedOrder makeBuy(ClientId id, order::Price price, order::Qty qty)
    {
        return order::AckedOrder{
            order::Order{
            .price = price,
            .qty = qty,
            .side = order::Side::Buy,
            .symbol = SYM,
            .clientId = id
        }, 
        nextOrderId()};
    }
    order::AckedOrder makeSell(ClientId id, order::Price price, order::Qty qty)
    {
        return order::AckedOrder{
            order::Order{
            .price = price,
            .qty = qty,
            .side = order::Side::Sell,
            .symbol = SYM,
            .foreignOrderId = nextOrderId(),
            .clientId = id 
            }, nextOrderId()};
    }
};

TEST_F(OrderBookTest, SimpleBuyAndSell)
{
    OrderBook book;

    order::Qty client1Qty = 10, client2Qty = 20;
    order::Price buyPrice = 15, sellPrice = 10;
    order::AckedOrder buy = makeBuy(client1Id, buyPrice, client1Qty);
    order::AckedOrder sell = makeSell(client2Id, sellPrice, client2Qty);

    EXPECT_TRUE(book.place(buy).empty());

    auto execs = book.place(sell);

    ASSERT_EQ(execs.size(), 1);
    EXPECT_EQ(execs.front().execPrice, sellPrice);
    EXPECT_EQ(execs.front().execQty, std::min(client1Qty, client2Qty));
}

TEST_F(OrderBookTest, NoMatchingTrade)
{
    OrderBook book;

    order::Qty client1Qty = 10, client2Qty = 20;
    
    // sellPrice > buyPrice
    order::Price buyPrice = 15, sellPrice = 20;

    order::AckedOrder buy = makeBuy(client1Id, buyPrice, client1Qty);
    order::AckedOrder sell = makeSell(client2Id, sellPrice, client2Qty);

    EXPECT_TRUE(book.place(buy).empty());
    EXPECT_TRUE(book.place(sell).empty());
}

TEST_F(OrderBookTest, MultipleFills)
{
    OrderBook book;

    order::Qty client1Qty = 10, client2FirstQty = 2, client2SecondQty = 15;
    
    // sellPrice > buyPrice
    order::Price buyPrice = 15, sellPrice = 10;

    order::AckedOrder buy = makeBuy(client1Id, buyPrice, client1Qty);

    order::AckedOrder sell1 = makeSell(client2Id, sellPrice, client2FirstQty);
    std::this_thread::sleep_for(5ms);
    order::AckedOrder sell2 = makeSell(client2Id, sellPrice, client2SecondQty);

    EXPECT_TRUE(book.place(sell1).empty());
    EXPECT_TRUE(book.place(sell2).empty());

    auto execs = book.place(buy);

    EXPECT_TRUE(std::any_of(execs.begin(), execs.end(),
        [&](order::Execution exec)
        {
            // total fill of first sell
            return exec.execPrice == buyPrice && exec.execQty == client2FirstQty;
        }
    ));
    EXPECT_TRUE(std::any_of(execs.begin(), execs.end(),
        [&](order::Execution exec)
        {
            // partial fill of second order
            return exec.execPrice == buyPrice && exec.execQty == client1Qty - client2FirstQty;
        }
    ));
}

}