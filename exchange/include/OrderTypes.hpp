#pragma once
#include <cstddef>
#include <variant>
#include <array>
#include <Types.hpp>
#include <string>

namespace exch::order {

/*
    @note: you shouldn't do this

    @todo: make a Price class that has arithmetic operations defined
    but has an integer representation to avoid float inaccuracies
*/
using Price = double;

/*
    Bloomberg encoding of the ticker e.x. AAPL

    In practice this is NOT only 4 bytes
    but for our purposes we assume that it is 
*/
using BBGTicker = std::string;

using Qty = std::size_t;
using Id = std::size_t;

enum class Type
{
    Limit
};
enum class Side
{
    Buy, Sell
};


struct Order
{
    Price price;
    Qty qty;
    order::Type type = order::Type::Limit;
    order::Side side;
    BBGTicker symbol;
    Timepoint sendTime;

    // this ID only gets used to identify the order to
    // the client. Upon receiving an order, we generate our own ID for
    // it and just use that!
    order::Id foreignOrderId;

    ClientId clientId;
};

/* Lambrusco */
struct AckedOrder : Order {
    order::Id exchangeId;
};

struct Ack
{
    order::Id clientId;
    order::Id vendorId;
};
struct Fill
{
    order::Id id;
    Price fillPrice;
    //@note: partial fills are allowed
    Qty fillQty;
};

/*
    An internal type (not sent back to a client)
*/
struct Execution
{
    AckedOrder side1;
    AckedOrder side2;

    order::Qty execQty;
    order::Price execPrice;
};

// Messages the client can send to us
using ClientOutboundMessages = std::variant<
    Order
>;

// Messages we may send to the client
using ServerOutboundMessages = std::variant<
    Fill
    , Ack
>;


}