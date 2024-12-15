#pragma once
#include <vector>
#include <chrono>
#include <unordered_map>
#include <asio/experimental/concurrent_channel.hpp>
#include <asio/experimental/channel.hpp>

namespace exch {

using PortNum = unsigned short;
using ClientId = size_t;
using Timepoint = std::chrono::steady_clock::time_point;

template<typename T>
using SmallVector = std::vector<T>;

template<typename K, typename V>
using FlatMap = std::unordered_map<K,V>;

template<typename T>
using channel = asio::experimental::channel<void(asio::error_code, T)>;


}