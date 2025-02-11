cmake_minimum_required(VERSION 3.10)

# Set project name
project(BitStack)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ensure source files exist
file(GLOB_RECURSE SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

# Add the executable
add_executable(BitStack ${SOURCES})

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)