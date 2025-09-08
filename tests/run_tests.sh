#!/bin/bash

# Simple script to build and run tests
echo "Building and running Multibind tests..."

# Check if we have gtest installed
if ! pkg-config --exists gtest; then
    echo "Installing Google Test..."
    sudo apt-get update
    sudo apt-get install -y libgtest-dev libgmock-dev cmake
    
    # Build and install gtest if needed
    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make
    sudo cp lib/*.a /usr/lib
fi

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make

# Run tests
if [ $? -eq 0 ]; then
    echo "Running tests..."
    ./multibind_tests
else
    echo "Build failed!"
    exit 1
fi