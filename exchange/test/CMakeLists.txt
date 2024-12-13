cmake_minimum_required(VERSION 3.22.1)
enable_testing()

add_executable(exchangeTests
    MatchingEngineTest.cpp
)

target_link_libraries(exchangeTests
    nlohmann_json::nlohmann_json
    GTest::gtest_main
    GTest::gmock_main)

include(GoogleTest)
gtest_discover_tests(exchangeTests)