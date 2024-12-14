#CMake_version
cmake_minimum_required(VERSION 3.10)

project("tools_test")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -O2")

add_executable(tools_test tools.cpp main.cpp)