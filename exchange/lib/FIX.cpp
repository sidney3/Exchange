#include "FIX.hpp"

namespace exch::FIX {

asio::awaitable<std::optional<Message>> readLogon(asio::ip::tcp::socket &sock) {
    throw std::runtime_error{"Not yet implemented!"};
}
asio::awaitable<std::optional<Message>> readOrder(asio::ip::tcp::socket &sock) {
    throw std::runtime_error{"Not yet implemented!"};
}

size_t encodeAck(Message &ack, std::vector<char> &out) {
    throw std::runtime_error{"Not yet implemented!"};
}
size_t encodeFill(Message &fill, std::vector<char> &out) {
    throw std::runtime_error{"Not yet implemented!"};
}

}