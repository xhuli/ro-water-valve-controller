# Standalone host-native project: this test builds and runs on the
# development machine (not the ATtiny85), so it needs its own project()
# rather than being add_subdirectory()'d from the top-level AVR cross build
# (see the comment there for why). Configure/build/run it independently:
#   cmake -S test -B test/cmake-build-debug && cmake --build test/cmake-build-debug && ctest --test-dir test/cmake-build-debug
#
# Uses CMake's normal default host compiler (gcc/g++ on Linux/macOS, or
# whatever toolset the active generator selects on Windows) rather than a
# hardcoded path.
cmake_minimum_required(VERSION 3.13)
project(ControllerTest CXX)

enable_testing()

add_executable(ControllerTest controllerTest.cpp)
add_test(NAME ControllerTest COMMAND ControllerTest)
