#pragma once
#include <fmt/format.h>
#include <cstddef>
#include <variant>
#include <array>
#include <Types.hpp>
#include <string>
#include <iostream>

namespace exch::order {

/*
    @note: you shouldn't do this

    @todo: make a Price class that has arithmetic operations defined
    but has an integer representation to avoid float inaccuracies
*/
using Price = double;

/*
    Bloomberg encoding of the ticker e.x. AAPL

    NOT very efficient to represent this as a string, but we do it anyways
*/
using BBGTicker = std::string;

using Qty = std::size_t;
using Id = std::size_t;

enum class Type
{
    Limit
};
std::ostream &operator<<(std::ostream &ostream, const Type& type);

enum class Side
{
    Buy, Sell
};
std::ostream &operator<<(std::ostream &ostream, const Side& side);



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
std::ostream &operator<<(std::ostream &ostream, const Order &order);


struct AckedOrder : Order {
    order::Id exchangeId;
};

struct Ack
{
    order::Id clientId;
    order::Id vendorId;
};

std::ostream &operator<<(std::ostream &ostream, const Ack &ack);

struct Fill
{
    order::Id id;
    Price fillPrice;
    //@note: partial fills are allowed
    Qty fillQty;
};

std::ostream &operator<<(std::ostream &ostream, const Fill &fill);

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