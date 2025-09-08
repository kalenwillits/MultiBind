cmake_minimum_required(VERSION 3.16)
project(MultibindTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(GTest REQUIRED)
find_package(GMock REQUIRED)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/../src)
include_directories(${CMAKE_SOURCE_DIR}/../SDK/CHeaders/XPLM)

# Add the source files we need to test
set(MULTIBIND_SOURCES
    ../src/combination_tracker.cpp
    ../src/config.cpp
)

# Create test executable
add_executable(multibind_tests
    test_combination_tracker.cpp
    ${MULTIBIND_SOURCES}
)

# Link libraries
target_link_libraries(multibind_tests 
    GTest::gtest 
    GTest::gtest_main
    GMock::gmock
)

# Enable testing
enable_testing()
add_test(NAME MultibindTests COMMAND multibind_tests)

# Compiler flags
target_compile_options(multibind_tests PRIVATE -Wall -Wextra)