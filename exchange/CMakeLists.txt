cmake_minimum_required(VERSION 3.22.1)

add_library(exchangeLib INTERFACE)
target_include_directories(exchangeLib INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(exchangeLib INTERFACE 
    asio 
)

add_library(exchange 
    src/ClientConnection.cpp
    src/Gateway.cpp
    src/MatchingEngine.cpp
    src/Order.cpp
    src/OrderBook.cpp
)

target_include_directories(exchange PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(exchange PUBLIC 
    exchangeLib
    asio 
    fmt
)

add_executable(testDriver
    src/TestDriver.cpp)

target_link_libraries(testDriver PRIVATE 
    exchange
)

set_target_properties(testDriver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_subdirectory(test)