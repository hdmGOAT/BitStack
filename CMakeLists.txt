cmake_minimum_required(VERSION 3.10)

# Set project name
project(BitStack)


find_package(OpenMP REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)

# Collect source files explicitly
set(SOURCES 
    ${CMAKE_SOURCE_DIR}/src/main.cpp 
    ${CMAKE_SOURCE_DIR}/src/bitstack/bitstack.cpp
)

# Add the executable

add_executable(btsk ${SOURCES})

if(OpenMP_CXX_FOUND)
    target_link_libraries(btsk PRIVATE OpenMP::OpenMP_CXX)
endif()
