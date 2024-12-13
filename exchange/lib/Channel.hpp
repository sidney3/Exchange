#pragma once
#include <asio.hpp>

namespace exch::lib {

/*
    Safe for one sending thread and one receiving thread (they may be distinct!)

    Note that this is the main performance concern
*/

template<typename T>
class NaiveChannel
{
public:
    [[nodiscard]] asio::awaitable<T> receive();
    void send(T);
};

template<typename T>
using Channel = NaiveChannel<T>;

}