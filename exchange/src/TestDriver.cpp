#include <asio.hpp>
#include <TestAsyncApplication.hpp>
#include <thread>
#include <chrono>

int main()
{
    asio::io_context context;
    AsyncApplication app;
    asio::co_spawn(context, app.run(), asio::detached);

    std::thread jt{[&](){
        context.run();
    }};

    std::this_thread::sleep_for(std::chrono::seconds(20));

    app.stop();
    context.stop();
    jt.join();
}