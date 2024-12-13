#pragma once
#include <asio.hpp>

namespace exch::lib {

template<typename T>
class MRMWChannel
{
public:
    asio::awaitable<T> receive();
    void send(T);
};

}