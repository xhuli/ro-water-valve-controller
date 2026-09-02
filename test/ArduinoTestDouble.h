#ifndef RO_WATER_VALVE_CONTROLLER_ARDUINO_TEST_DOUBLE_H
#define RO_WATER_VALVE_CONTROLLER_ARDUINO_TEST_DOUBLE_H

#include <cstdint>

/**
 * Minimal host-side stand-in for the slice of the Arduino API that
 * src/main/Controller.h depends on: pinMode/digitalWrite/digitalRead/
 * millis/delay plus the HIGH/LOW/OUTPUT/INPUT_PULLUP constants. Lets the
 * real production classes run, and be driven with a fabricated clock, on
 * the development machine with no hardware and no AVR toolchain.
 *
 * Not a general Arduino emulator - just enough surface for Controller.h.
 */

constexpr uint8_t LOW = 0;
constexpr uint8_t HIGH = 1;
constexpr uint8_t INPUT = 0;
constexpr uint8_t OUTPUT = 1;
constexpr uint8_t INPUT_PULLUP = 2;

namespace test_double
{
    constexpr uint8_t PIN_COUNT = 32;

    inline uint8_t pinState[PIN_COUNT] = {};
    inline uint8_t pinModeState[PIN_COUNT] = {};
    inline uint32_t fakeMillis = 0;

    /** Resets all fake pin state and the fake clock between tests. */
    inline void reset()
    {
        fakeMillis = 0;
        for (uint8_t &state : pinState)
        {
            state = LOW;
        }
        for (uint8_t &mode : pinModeState)
        {
            mode = INPUT;
        }
    }
}

inline void pinMode(uint8_t pin, uint8_t mode)
{
    test_double::pinModeState[pin] = mode;
}

inline void digitalWrite(uint8_t pin, uint8_t value)
{
    test_double::pinState[pin] = value;
}

inline uint8_t digitalRead(uint8_t pin)
{
    return test_double::pinState[pin];
}

inline uint32_t millis()
{
    return test_double::fakeMillis;
}

inline void delay(uint32_t)
{
    /* no-op: tests advance test_double::fakeMillis explicitly instead */
}

#endif
