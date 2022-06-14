cmake_minimum_required(VERSION 3.13)

#SET(CMAKE_C_COMPILER "/usr/bin/cc")
#SET(CMAKE_CXX_COMPILER "/usr/bin/c++")
SET(CMAKE_C_COMPILER "c:/bin/mingw64/bin/gcc.exe")
SET(CMAKE_CXX_COMPILER "c:/bin/mingw64/bin/g++.exe")

add_executable(BuzzerTest buzzerTest.cpp)
add_test(NAME BuzzerTest COMMAND BuzzerTest)
