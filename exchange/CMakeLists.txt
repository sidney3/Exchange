cmake_minimum_required(VERSION 3.22.1)

add_library(exchangeLib 
    lib/Fix.cpp
)
target_include_directories(exchangeLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)
target_link_libraries(exchangeLib PRIVATE
    asio 
)

add_library(exchange 
    src/ClientConnection.cpp
    src/Gateway.cpp
    src/MatchingEngine.cpp
    src/Order.cpp
    src/OrderBook.cpp
    src/EnginePort.cpp
    src/Exchange.cpp
    src/OrderTypes.cpp
)

target_include_directories(exchange PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_link_libraries(exchange PUBLIC 
    # Sick Google data structures
    absl::base          
    absl::strings       
    absl::flat_hash_map  
    absl::inlined_vector
    exchangeLib
    asio 
    fmt
)

add_executable(testDriver
    src/TestDriver.cpp)
target_link_libraries(testDriver PRIVATE 
    exchange
)

add_executable(client
    exampleClient/client.cpp
)
target_link_libraries(client PRIVATE 
    exchange
    fmt
)

add_executable(driver
    src/Driver.cpp
)
target_link_libraries(driver PRIVATE 
    exchange
)


set_target_properties(testDriver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(driver PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

add_subdirectory(test)
add_subdirectory(benchmark)