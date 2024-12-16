#include <fmt/format.h>
#include <FIX.hpp>
#include <asio.hpp>
#include <iostream>

int main(int argc, char **argv) {
    if(argc != 3)
    {
        fmt::println("Usage: ./client <exchangeAddress> <exchangePort>");
        return 0;
    }

    asio::io_context context;

    auto addr = asio::ip::make_address_v4(argv[1]);
    unsigned short portNum = std::stoi(argv[2]);

    asio::ip::tcp::endpoint ep{addr, portNum};
    asio::ip::tcp::socket sock{context};
    asio::error_code ec;
    sock.connect(ep, ec);

    if(ec) {
        fmt::println("Error connecting to server: {}", ec.message());
        return 0;
    }

    std::string userInput;

    constexpr size_t maxMessageSize = 1024;
    std::vector<char> readWriteBuffer(maxMessageSize);
    constexpr char writeCmd = 'W', readCmd = 'R';

    while(true) {
        std::cout << ">";
        std::getline(std::cin, userInput);

        if(userInput.empty()) {
            continue;
        }
        switch(userInput.front())
        {
        case writeCmd: {
            if(userInput.size() > 2) {
                size_t messageSendSize = userInput.size() - 2;
                if(messageSendSize > maxMessageSize)
                {
                    fmt::println("Message of length {} exceeds max length of {}", messageSendSize, maxMessageSize);
                    break;
                }
                std::copy(std::next(userInput.begin(), 2), userInput.end(), readWriteBuffer.begin());
                asio::write(sock, asio::buffer(readWriteBuffer, messageSendSize));
            }
            break;
        }
        case readCmd: {
            size_t readSize = sock.read_some(asio::buffer(readWriteBuffer));
            for(size_t i = 0; i < readSize; ++i)
            {
                std::cout << readWriteBuffer[i];
            }
            std::cout << std::endl;
        }
        }

    }
}