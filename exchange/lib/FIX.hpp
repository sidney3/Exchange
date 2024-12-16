#include <asio.hpp>
#include <optional>
#include <unordered_map>

namespace exch::FIX {

/*
    @note: this is about as inefficient of a FIX encoding as we can do.
    But that's not really the focus of this project.

*/
using fix_key_t = std::size_t;
using Message = std::unordered_map<fix_key_t, std::string>;

constexpr char delim = '|';

struct FixField
{
    static constexpr fix_key_t check = 10;
};

constexpr size_t MaxMessageSize = 4096;

asio::awaitable<std::optional<Message>> readLogon(asio::ip::tcp::socket &sock);
asio::awaitable<std::optional<Message>> readOrder(asio::ip::tcp::socket &sock);

size_t encodeAck(Message &ack, std::vector<char> &out);
size_t encodeFill(Message &fill, std::vector<char> &out);
size_t encodeLogon(std::vector<char> &out);

}