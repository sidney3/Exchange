#pragma once
#include <cstddef>
#include <variant>
#include <array>

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
using BBGTicker = std::array<char, 4>;

using Qty = std::size_t;
using Id = std::size_t;

enum class OrderType
{
    Limit
};
enum class OrderSide
{
    Buy, Sell
};


struct Order
{
    Price price;
    OrderType type;
    OrderSide side;
    BBGTicker symbol;

    // this ID only gets used to identify the order to
    // the client. Upon receiving an order, we generate our own ID for
    // it and just use that!
    Id clientId;
};

struct Ack
{
    Id clientId;
    Id vendorId;
};
struct Fill
{
    Id id;
    Price fillPrice;
    //@note: partial fills are allowed
    Qty fillQty;
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