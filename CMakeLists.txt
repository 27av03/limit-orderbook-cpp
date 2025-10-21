cmake_minimum_required(VERSION 3.15)
project(OrderBook CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Add main executable
add_executable(orderbook_main
    src/main.cpp
    src/OrderBook.cpp
)

# Add test executables (without GTest for now)
add_executable(orderbook_tests
    tests/simple_test.cpp
    src/OrderBook.cpp
)

