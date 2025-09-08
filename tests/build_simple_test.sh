#!/bin/bash

echo "Building simple test..."

# Compile the test with the source files we need
g++ -std=c++17 -I../src -I../SDK/CHeaders/XPLM \
    simple_test.cpp \
    ../src/combination_tracker.cpp \
    ../src/config.cpp \
    -o simple_test

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "Running test..."
    echo ""
    ./simple_test
else
    echo "❌ Build failed!"
    exit 1
fi