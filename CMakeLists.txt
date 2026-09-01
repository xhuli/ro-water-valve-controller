cmake_minimum_required(VERSION 3.13)

SET(MCU "attiny85")
# NOTE / discrepancy: claude_custom_instructions.md documents "clock
# assumption: 8 MHz" and the previous version of this file used F_CPU
# 8000000. But the board's own PlatformIO definition (digispark-tiny.json,
# authoritative per that same doc) sets f_cpu to 16500000L, and `pio run`
# builds with that value with no override in platformio.ini. Matched here so
# this CMake build reflects the same clock the real firmware is built for;
# flagging this instead of silently leaving it, since the docs disagree with
# the actual board default and that's worth a deliberate decision, not a
# silent fix.
SET(F_CPU "16500000")
SET(CMAKE_SYSTEM_NAME Generic)

# AVR toolchain + Arduino core headers: reuse PlatformIO's own packages
# instead of guessing an Arduino IDE install path. PlatformIO is already
# required for this project (see platformio.ini) and installs identically
# under ~/.platformio on Linux/macOS and %USERPROFILE%\.platformio on
# Windows, so this works cross-platform with no per-OS branching needed.
if (WIN32)
    set(PIO_HOME "$ENV{USERPROFILE}/.platformio")
else ()
    set(PIO_HOME "$ENV{HOME}/.platformio")
endif ()

set(AVR_TOOLCHAIN_DIR "${PIO_HOME}/packages/toolchain-atmelavr/bin")
set(AVR_FRAMEWORK_DIR "${PIO_HOME}/packages/framework-arduino-avr-digistump")

find_program(AVR_GCC_PATH avr-gcc HINTS "${AVR_TOOLCHAIN_DIR}" PATHS ENV PATH)
find_program(AVR_GXX_PATH avr-g++ HINTS "${AVR_TOOLCHAIN_DIR}" PATHS ENV PATH)

if (NOT AVR_GCC_PATH OR NOT AVR_GXX_PATH)
    message(FATAL_ERROR
            "AVR toolchain not found. Run `pio run -e usbasp` once so "
            "PlatformIO installs toolchain-atmelavr under ${PIO_HOME}, "
            "or install a system AVR toolchain (e.g. `apt install gcc-avr avr-libc` on Ubuntu).")
endif ()

SET(CMAKE_C_COMPILER "${AVR_GCC_PATH}")
SET(CMAKE_CXX_COMPILER "${AVR_GXX_PATH}")

# CMAKE_SYSTEM_NAME/CMAKE_C_COMPILER/CMAKE_CXX_COMPILER are fixed for the
# whole build tree the moment project() runs; setting them any later (as
# this file used to) is silently ignored and CMake falls back to the host
# compiler. Must come before project().
project(RoWaterController)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# -ffunction-sections/-fdata-sections + linker --gc-sections (below) mirror
# what PlatformIO actually does (verified via `pio run -e usbasp -v`) and are
# not just an optimization here: without them the whole Arduino core links
# in unconditionally (Tone, all 4 TinyDebugSerial baud variants, etc.) and
# overflows the ATtiny85's 6012-byte flash budget by ~9KB.
SET(CMAKE_C_FLAGS "-mmcu=${MCU} -DF_CPU=${F_CPU} -Os -ffunction-sections -fdata-sections")
# CXX must get the same -mmcu: it's what makes avr-gcc define __AVR_ATtinyX5__
# etc. internally, which the core's headers (e.g. core_build_options.h,
# TinyDebugSerial.h) branch on. C-only flags left this undefined for every
# .cpp file and broke the build.
SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
SET(CMAKE_C_LINK_FLAGS "-mmcu=${MCU}")
SET(CMAKE_CXX_LINK_FLAGS "-mmcu=${MCU}")
SET(CMAKE_EXE_LINKER_FLAGS "-mmcu=${MCU} -Os -Wl,--gc-sections")

SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Board is digispark-tiny (see platformio.ini), so the Arduino core is
# framework-arduino-avr-digistump, not the stock "arduino" core.
include_directories("${AVR_FRAMEWORK_DIR}/cores/dtiny")
include_directories("${AVR_FRAMEWORK_DIR}/variants/digispark")

# Header include paths alone aren't enough to link: millis()/pinMode()/etc.
# are only declared in the headers above, defined in these core sources.
# PlatformIO compiles this same set (verified via `pio run -e usbasp -v`)
# plus the board's pins_arduino.c; mirror that here so the CMake build
# actually links instead of just serving CLion code navigation.
#
# Built as a STATIC library rather than compiled straight into the
# executable: a static lib only pulls in the .o members it actually
# resolves symbols from, same as PlatformIO's libFrameworkArduino.a. Passed
# as direct sources instead, everything (Tone, all TinyDebugSerial baud
# variants, ...) gets linked in unconditionally and overflows flash.
file(GLOB AVR_CORE_SOURCES
        "${AVR_FRAMEWORK_DIR}/cores/dtiny/*.c"
        "${AVR_FRAMEWORK_DIR}/cores/dtiny/*.cpp")
list(APPEND AVR_CORE_SOURCES "${AVR_FRAMEWORK_DIR}/variants/digispark/pins_arduino.c")
add_library(FrameworkArduino STATIC ${AVR_CORE_SOURCES})

# test/ is NOT add_subdirectory()'d here: it builds and runs on the host
# (needs <iostream>/host STL), but CMAKE_C_COMPILER/CMAKE_CXX_COMPILER above
# are fixed for this entire configure once project() runs, so it can't share
# a configure with the AVR-cross-compiled target. Build it as its own
# standalone project instead:
#   cmake -S test -B test/build && cmake --build test/build && ctest --test-dir test/build

add_executable(RodiController src/main/main.cpp)
target_link_libraries(RodiController PRIVATE FrameworkArduino m)
set_target_properties(RodiController PROPERTIES LINKER_LANGUAGE CXX)
