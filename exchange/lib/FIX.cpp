#include "FIX.hpp"
#include <iostream>

namespace exch::FIX {

asio::awaitable<std::pair<fix_key_t, std::string>> readOne(asio::ip::tcp::socket &sock) {
    std::string keyStr, valStr;

    std::array<char, 1> buf;
    while(co_await asio::async_read(sock, asio::buffer(buf,1), asio::use_awaitable), buf[0] != '=') {
        keyStr += buf[0];
    }
    while(co_await asio::async_read(sock, asio::buffer(buf,1), asio::use_awaitable), buf[0] != delim) {
        valStr += buf[0];
    }

    co_return std::pair<fix_key_t, std::string>{std::stoul(keyStr), valStr};
}

// Reads a FIX message from the socket
asio::awaitable<std::optional<Message>> readFIXMessage(asio::ip::tcp::socket &sock) {
    Message msg;
    while(true)
    {
        auto [key, val] = co_await readOne(sock);
        msg[key] = std::move(val);
        if(key == FixField::check)
        {
            break;
        }
    }
    co_return msg;
}

asio::awaitable<std::optional<Message>> readLogon(asio::ip::tcp::socket &sock) {
    auto messageOpt = co_await readFIXMessage(sock);
    if (messageOpt && messageOpt->count(35) && (*messageOpt)[35] == "A") { // 35=A indicates a Logon message
        co_return messageOpt;
    }
    co_return std::nullopt;
}

asio::awaitable<std::optional<Message>> readOrder(asio::ip::tcp::socket &sock) {
    auto messageOpt = co_await readFIXMessage(sock);
    if (messageOpt && messageOpt->count(35) && (*messageOpt)[35] == "D") { // 35=D indicates a New Order Single message
        co_return messageOpt;
    }
    co_return std::nullopt;
}

size_t encodeMessage(const Message &msg, std::vector<char> &out) {
    std::ostringstream stream;
    for (const auto &[key, value] : msg) {
        stream << key << '=' << value << '\x01'; // SOH (\x01) is the standard delimiter
    }
    std::string encoded = stream.str();
    out.insert(out.end(), encoded.begin(), encoded.end());
    return encoded.size();
}

size_t encodeAck(Message &ack, std::vector<char> &out) {
    ack[35] = "8"; // 35=8 is an Execution Report, used for Ack
    return encodeMessage(ack, out);
}

size_t encodeFill(Message &fill, std::vector<char> &out) {
    fill[35] = "8"; // 35=8 is also used for fills with appropriate status
    fill[150] = "2"; // 150=2 indicates a Fill
    fill[39] = "2";  // 39=2 indicates Order Status as Filled
    return encodeMessage(fill, out);
}

size_t encodeLogon(std::vector<char> &out) {
    const std::string mockLogon = "8=FIX.4.4|9=112|35=A|34=1|49=EXCHANGE|56=CLIENT|98=0|108=30|141=Y|553=USERNAME|554=PASSWORD|10=128|";
    if(out.size() < mockLogon.size())
    {
        return -1;
    }
    std::copy(mockLogon.begin(), mockLogon.end(), out.begin());
    return mockLogon.size();
}

}