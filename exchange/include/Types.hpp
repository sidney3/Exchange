#pragma once
#include <vector>
#include <chrono>
#include <unordered_map>
#include <asio/experimental/concurrent_channel.hpp>
#include <asio/experimental/channel.hpp>
#include <absl/container/flat_hash_map.h>
#include <absl/container/inlined_vector.h>

namespace exch {

using PortNum = unsigned short;
using ClientId = size_t;
using Timepoint = std::chrono::steady_clock::time_point;

/* We usually expect to get about 1 execution per order */
template<typename T>
using SmallVector = std::vector<T>;

template<typename K, typename V>
using FlatMap = absl::flat_hash_map<K,V>;

template<typename T>
using channel = asio::experimental::channel<void(asio::error_code, T)>;

template<typename ... Ts>
struct Overloaded : Ts...
{
    using Ts::operator()...;
};
template<typename ... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

}