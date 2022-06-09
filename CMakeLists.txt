cmake_minimum_required(VERSION 3.13)
project(RoWaterController)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

SET(MCU "attiny85")
SET(F_CPU "8000000")
SET(CMAKE_SYSTEM_NAME Generic)

#SET(ARDUINO_DIR "c:\\Program Files (x86)\\Arduino")
SET(ARDUINO_DIR "/usr/share/arduino")
# For some reason, these paths have to be absolute, otherwise CLion won't be able to find headers etc.
#SET(CMAKE_C_COMPILER "${ARDUINO_DIR}\\hardware\\tools\\avr\\bin\\avr-gcc.exe")
#SET(CMAKE_CXX_COMPILER "${ARDUINO_DIR}\\hardware\\tools\\avr\\bin\\avr-g++.exe")
SET(CMAKE_C_COMPILER "/usr/bin/avr-gcc")
SET(CMAKE_CXX_COMPILER "/usr/bin/avr-g++")


SET(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

add_library(ArduinoLibraries SHARED IMPORTED include/Buzzer.h)
#include_directories("${ARDUINO_DIR}\\hardware\\arduino\\avr\\cores\\arduino")
#include_directories("${ARDUINO_DIR}\\hardware\\tools\\avr\\avr\\include\\avr")
include_directories("${ARDUINO_DIR}/hardware/arduino/avr/cores/arduino")
include_directories("${ARDUINO_DIR}/hardware/tools/avr/avr/include/avr")

include_directories(include)

enable_testing()
add_subdirectory(test)

add_executable(RodiController src/main.cpp)
set_target_properties(RodiController PROPERTIES LINKER_LANGUAGE CXX)
