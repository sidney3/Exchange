#include "EnginePort.hpp"
#include <gtest/gtest.h>
#include <asio.hpp>
#include <MatchingEngine.hpp>
#include <thread>

namespace exch { 

class MatchingEngineTest : public ::testing::Test
{
protected:
    MatchingEngineTest() {
        asio::co_spawn(context, matchingEngine.run(), asio::detached);
    }

    /*
        For testing related coroutines that we would like to spawn and synchronously wait for 
    */
    asio::io_context testingContext{};
    std::jthread testingThread{[&](){
        testingContext.run();
    }};


    asio::io_context context{};
    std::jthread executorThread{[&](){
        context.run();
    }};
    ExchangeConfig testConfig{};
    MatchingEngine matchingEngine{testConfig};
    asio::experimental::generator<EnginePort> portGen = matchingEngine.connect();

    EnginePort getPort()
    {
        auto port = asio::co_spawn(testingContext, portGen.async_resume(asio::use_awaitable), asio::use_future).get();
        if(port.has_value())
        {
            return *port;
        }
        else
        {
            throw std::runtime_error{"Error accessing EnginePort"};
        }
    }
};

TEST_F(MatchingEngineTest, ExampleTest)
{
    EnginePort enginePort = getPort();
    EXPECT_EQ(0,0);
}

}