#include "OrderTypes.hpp"
#include <fmt/format.h>

namespace exch::order {

std::ostream &operator<<(std::ostream &ostream, const Type& type) {
    switch(type)
    {
    case Type::Limit: {
        ostream << "Limit";
        break;
    }
    default: {
        ostream << "Unrecognized type";
        break;
    }
    }
    return ostream;
}
std::ostream &operator<<(std::ostream &ostream, const Side& side) {
    switch(side)
    {
    case Side::Buy: {
        ostream << "Buy";
        break;
    }
    case Side::Sell: {
        ostream << "Sell";
        break;
    }
    default: {
        ostream << "Unrecognized side";
        break;
    }
    }
    return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const Order &order) {
    ostream << "["
            << "Price: " << order.price << ", "
            << "Qty: " << order.qty << ", "
            << "Side: " << order.side << ", "
            << "Symbol: " << order.symbol << ", "
            << "ClientId: " << order.clientId << ""
            << "]";
    return ostream;
}

std::ostream &operator<<(std::ostream &ostream, const Ack &ack) {
    ostream << "["
            << "Id: " << ack.vendorId 
            << "]";
    return ostream;

}

std::ostream &operator<<(std::ostream &ostream, const Fill &fill) {
    ostream << "["
            << "Id: " << fill.id << ", "
            << "fillPrice: " << fill.fillPrice<< ", "
            << "fillQty: " << fill.fillQty
            << "]";

    return ostream;
}

}