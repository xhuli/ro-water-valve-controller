# Standalone host-native project: this test builds and runs on the
# development machine (not the ATtiny85), so it needs its own project()
# rather than being add_subdirectory()'d from the top-level AVR cross build
# (see the comment there for why). Configure/build/run it independently:
#   cmake -S test -B test/build && cmake --build test/build && ctest --test-dir test/build
#
# Uses CMake's normal default host compiler (gcc/g++ on Linux/macOS, or
# whatever toolset the active generator selects on Windows) rather than a
# hardcoded path.
cmake_minimum_required(VERSION 3.13)
project(BuzzerTest CXX)

enable_testing()

add_executable(BuzzerTest buzzerTest.cpp)
add_test(NAME BuzzerTest COMMAND BuzzerTest)
