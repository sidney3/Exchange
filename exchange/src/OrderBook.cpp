#include "OrderBook.hpp"
#include "OrderTypes.hpp"
#include "Types.hpp"

namespace exch {

bool SymbolBook::BuyComparator::operator()(order::AckedOrder order1, order::AckedOrder order2)
{
    return (order1.price > order2.price)
        || (order1.price == order2.price && order1.sendTime < order2.sendTime);
}

bool SymbolBook::SellComparator::operator()(order::AckedOrder order1, order::AckedOrder order2)
{
    return (order1.price < order2.price)
        || (order1.price == order2.price && order1.sendTime < order2.sendTime);
}


SmallVector<order::Execution> SymbolBook::place(order::AckedOrder order) {
    if(order.side == order::Side::Buy)
    {
        return placeBuy(std::move(order));
    }
    else
    {
        return placeSell(std::move(order));
    }
}

SmallVector<order::Execution> SymbolBook::placeBuy(order::AckedOrder order) {
    return placeImpl(sells, buys, [](const order::Order &myOrder, const order::Order &theirOrder) {
        return myOrder.price >= theirOrder.price;
    }, std::move(order));
}

SmallVector<order::Execution> SymbolBook::placeSell(order::AckedOrder order) {
    return placeImpl(buys, sells, [](const order::Order &myOrder, const order::Order &theirOrder) {
        return myOrder.price <= theirOrder.price;
    }, std::move(order));
}

SmallVector<order::Execution> OrderBook::place(order::AckedOrder order) {
    return perSymbolBook[order.symbol].place(order);
}

}