#pragma once
#include <OrderTypes.hpp>
#include <Types.hpp>

namespace exch {


class OrderBook
{
public:
    struct Execution
    {
        order::Id sellId;
        order::Id buyId;

        order::Qty execQty;
        order::Price execPrice;
    };

    SmallVector<Execution> place(order::Order order, order::Id id);
};

}