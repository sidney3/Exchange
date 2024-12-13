#pragma once
#include <OrderTypes.hpp>
#include <Types.hpp>

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

class OrderBook
{
public:
    SmallVector<order::Execution> place(order::Order order, order::Id id);
};

}