#include "FIX.hpp"

namespace exch::FIX {

Message parseFIXMessage(const std::string &fixMessage) {
    Message msg;
    std::istringstream stream(fixMessage);
    std::string pair;
    while (std::getline(stream, pair, '\x01')) { // Fields are delimited by SOH (\x01)
        auto delimiterPos = pair.find('=');
        if (delimiterPos != std::string::npos) {
            fix_key_t key = std::stoul(pair.substr(0, delimiterPos));
            std::string value = pair.substr(delimiterPos + 1);
            msg[key] = value;
        }
    }
    return msg;
}

// Reads a FIX message from the socket
asio::awaitable<std::optional<Message>> readFIXMessage(asio::ip::tcp::socket &sock) {
    std::vector<char> buffer(MaxMessageSize);
    std::size_t bytesRead = co_await asio::async_read_until(sock, asio::dynamic_buffer(buffer), '\x01', asio::use_awaitable);

    if (bytesRead > 0) {
        std::string fixMessage(buffer.data(), bytesRead);
        co_return parseFIXMessage(fixMessage);
    }

    co_return std::nullopt;
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

}