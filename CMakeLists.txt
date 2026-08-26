cmake_minimum_required(VERSION 3.20)

project(cpp_trading_engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(trading_engine
    src/main.cpp
    src/price_level.cpp
    src/order_book.cpp
    src/matching_engine.cpp
    src/trading_server.cpp
)

target_include_directories(trading_engine
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)