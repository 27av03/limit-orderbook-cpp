#!/bin/bash
# Unix/Linux build script for OrderBook project

set -e  # Exit on any error

echo "Building OrderBook project..."

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# Build the project
echo "Building project..."
make -j$(nproc)
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Build completed successfully!"
echo

# Run tests
echo "Running basic tests..."
./orderbook_tests

echo
echo "Running comprehensive tests..."
./orderbook_comprehensive_tests

echo
echo "Running demo..."
./orderbook_main

cd ..
echo "All done!"
