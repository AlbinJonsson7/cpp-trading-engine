cmake_minimum_required(VERSION 3.20)

project(cpp_trading_engine
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


# =====================================================
# TRADING ENGINE CORE LIBRARY
# =====================================================

add_library(trading_core STATIC
    src/price_level.cpp
    src/order_book.cpp
    src/matching_engine.cpp
    src/order_index.cpp
    src/order_node_pool.cpp
)

target_include_directories(trading_core
    PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)


# =====================================================
# MAIN / DEMO EXECUTABLE
# =====================================================

add_executable(trading_engine
    src/main.cpp
)

target_link_libraries(trading_engine
    PRIVATE
    trading_core
)


# =====================================================
# CORRECTNESS TESTS
# =====================================================

add_executable(trading_engine_tests
    tests/trading_engine_tests.cpp
)

target_link_libraries(trading_engine_tests
    PRIVATE
    trading_core
)


# =====================================================
# BENCHMARKS
# =====================================================

add_executable(matching_engine_benchmark
    benchmarks/matching_engine_benchmark.cpp
)

target_link_libraries(matching_engine_benchmark
    PRIVATE
    trading_core
)


# =====================================================
# CTEST
# =====================================================

enable_testing()

add_test(
    NAME trading_engine_correctness
    COMMAND trading_engine_tests
)