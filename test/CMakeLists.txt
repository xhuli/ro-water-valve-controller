cmake_minimum_required(VERSION 3.13)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

add_executable(BuzzerTest buzzerTest.cpp)
add_test(NAME BuzzerTest COMMAND BuzzerTest)
