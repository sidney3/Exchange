#include "FIX.hpp"

namespace exch::FIX {

asio::awaitable<std::optional<Message>> readLogon(asio::ip::tcp::socket &sock) {

}
asio::awaitable<std::optional<Message>> readOrder(asio::ip::tcp::socket &sock) {

}

size_t encodeAck(Message &ack, std::vector<char> &out) {

}
size_t encodeFill(Message &fill, std::vector<char> &out) {

}

}