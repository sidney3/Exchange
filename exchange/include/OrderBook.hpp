#pragma once
#include <OrderTypes.hpp>
#include <Types.hpp>
#include <queue>
#include <functional>

namespace exch {

/*
    A classic DSA problem. 

    Fire in trades and generate executions.

    ************************************
    HOW DO WE HANDLE OVERLAPPING ORDERS?
    ************************************

    We will take the price of the aggressor (i.e. the party that is disturbing a resting order)

    So if our book holds 
    
    SELL AAPL @ 5

    And someone comes in with

    BUY AAPL @ 6

    The executed transaction would be to sell AAPL @ 6 (the price of the aggressor)
*/

class SymbolBook
{
private:
    /*
        @note we store the order::AckedOrder struct, and update the Quantity field to reflect the quantity remaining. 
    */
    struct BuyComparator
    {
        bool operator()(order::AckedOrder, order::AckedOrder);
    };
    struct SellComparator
    {
        bool operator()(order::AckedOrder, order::AckedOrder);
    };
    using order_storage_t = order::AckedOrder;
    std::priority_queue<order_storage_t, std::vector<order_storage_t>, BuyComparator> buys;
    std::priority_queue<order_storage_t, std::vector<order_storage_t>, SellComparator> sells;

    SmallVector<order::Execution> placeBuy(order::AckedOrder order);
    SmallVector<order::Execution> placeSell(order::AckedOrder order);

    SmallVector<order::Execution> placeImpl(auto &&myPq, auto &&theirPq, auto &&ordersMatch /* bool(myOrder, theirOrder)*/, order::AckedOrder order);
public:
    SmallVector<order::Execution> place(order::AckedOrder order);

    /* In a real exchange context we don't want self matches, but it messes with our benchmarks if we don't allow them */
    static constexpr bool allowSelfMatches = true;
};

class OrderBook
{
private:
    FlatMap<order::BBGTicker, SymbolBook> perSymbolBook;
public:
    SmallVector<order::Execution> place(order::AckedOrder order);
};

SmallVector<order::Execution> SymbolBook::placeImpl(
    auto &&myPq
    , auto &&theirPq
    , auto &&ordersMatch /* bool(myOrder, theirOrder)*/
    , order::AckedOrder order)
{
    auto canMatchOrders = [&](const order::AckedOrder &order)
    {
        return order.qty > 0 && !theirPq.empty() && ordersMatch(order, theirPq.top());
    };

    SmallVector<order::AckedOrder> selfMatches;
    SmallVector<order::Execution> executions;

    while(canMatchOrders(order))
    {
        order::AckedOrder matchedOrder = theirPq.top();
        theirPq.pop();
        
        if(!allowSelfMatches)
        {
            if(matchedOrder.clientId == order.clientId)
            {
                selfMatches.emplace_back(std::move(matchedOrder));
                continue;
            }
        }

        order::Qty execQty = std::min(matchedOrder.qty, order.qty);

        executions.push_back(order::Execution{
            .side1 = matchedOrder,
            .side2 = order,
            .execQty = execQty,
            .execPrice = order.price
        });

        order.qty -= execQty;
        matchedOrder.qty -= execQty;

        if(matchedOrder.qty > 0)
        {
            theirPq.push(matchedOrder);
        }
    }
    
    if(!allowSelfMatches)
    {
        /*
            Replace the self matches we found 
        */
        for(order::AckedOrder &selfMatch : selfMatches)
        {
            theirPq.push(std::move(selfMatch));
        }
    }

    if(order.qty > 0)
    {
        myPq.push(order);
    }

    return executions;
}

}