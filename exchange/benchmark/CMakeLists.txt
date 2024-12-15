set(SOURCES 
    MatchingEngineBenchmark.cpp
)

add_executable(ExchangeBenchmark ${SOURCES})
target_link_libraries(ExchangeBenchmark
    fmt
    exchange
    benchmark::benchmark
    pthread
)

target_compile_options(ExchangeBenchmark PRIVATE -O2)
set_target_properties(ExchangeBenchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")