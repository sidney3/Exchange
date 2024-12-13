cmake_minimum_required(VERSION 3.22.1)

project(exchange)

enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_Declare(
  asio
  URL https://github.com/chriskohlhoff/asio/archive/f88ec15630b71e154470659f6d5a2ef8e03bea2e.zip
)
FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/archive/63258397761b3dd96dd171e5a5ad5aa915834c35.zip
)
FetchContent_Declare(
  fmt
  URL https://github.com/fmtlib/fmt/archive/refs/heads/master.zip
)

FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(asio)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(fmt)

add_library(asio INTERFACE)
target_compile_options(asio INTERFACE -DASIO_STANDALONE)
target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

add_subdirectory(exchange)