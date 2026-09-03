/**
 * Host-side regression tests for the production classes under include/,
 * driven with a fabricated clock via ArduinoTestDouble.h instead of real
 * hardware/millis(). The actual Buzzer/SwitchableWithDelay/WaterLevelSensor
 * code is exercised directly, not reimplemented - this replaces the old
 * buzzerTest.cpp, which reimplemented Buzzer from scratch and so provided
 * no protection against bugs in the production class.
 *
 * Behaviors covered (see BACKLOG.md / claude_custom_instructions.md §11):
 *   - buzzer starts, repeats, and stops
 *   - timing behaves correctly across millis() rollover
 *   - 16-second-style anti-chatter delay works in both directions
 *   - immediate shutdown (switchOnNow/switchOffNow) bypasses the delay
 *   - sensor polarity (isNotSensingWater)
 *
 * Plain assert()-based, no test framework dependency - consistent with the
 * rest of this project's "avoid unnecessary dependencies" convention.
 */
#include <cassert>
#include <iostream>

#include "ArduinoTestDouble.h"
#include "../include/Buzzer.h"
#include "../include/SwitchableWithDelay.h"
#include "../include/WaterLevelSensor.h"

using test_double::fakeMillis;
using test_double::pinState;

// Fake pin numbers these tests construct their fixtures on. Arbitrary (the
// test double has 32 fake pins and doesn't care), but named rather than
// inlined so e.g. `digitalRead(5)` reads as "the buzzer's pin" instead of a
// bare number the reader has to trace back to a constructor call above.
constexpr uint8_t TEST_BUZZER_PIN = 5;
constexpr uint8_t TEST_VALVE_PIN = 6;
constexpr uint8_t TEST_SENSOR_PIN = 7;

static void test_buzzer_starts_repeats_and_stops()
{
    test_double::reset();
    // N must be even (see Buzzer.h): with an odd count, the wrap from the
    // last interval back to interval 0 is itself a toggle, so index 0 would
    // mean "stay LOW" on this lap instead of "stay HIGH" like the first -
    // exactly the confusion this test would otherwise be validating as
    // correct instead of catching.
    const uint32_t intervals[4] = {100, 200, 300, 150};
    Buzzer<4> buzzer(TEST_BUZZER_PIN, intervals);
    buzzer.setup();

    assert(digitalRead(TEST_BUZZER_PIN) == LOW);

    buzzer.startBuzzing(); // t=0: turns pin HIGH, enters interval 0 (100ms)
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    fakeMillis = 99;
    buzzer.startBuzzing(); // still within interval 0
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    fakeMillis = 100;
    buzzer.startBuzzing(); // interval 0 elapsed -> toggle, enter interval 1 (200ms)
    assert(digitalRead(TEST_BUZZER_PIN) == LOW);

    fakeMillis = 100 + 200;
    buzzer.startBuzzing(); // interval 1 elapsed -> toggle, enter interval 2 (300ms)
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    fakeMillis = 100 + 200 + 300;
    buzzer.startBuzzing(); // interval 2 elapsed -> toggle, enter interval 3 (150ms)
    assert(digitalRead(TEST_BUZZER_PIN) == LOW);

    fakeMillis = 100 + 200 + 300 + 150;
    buzzer.startBuzzing(); // interval 3 elapsed -> wraps back to interval 0, HIGH again (same phase as the first lap)
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    buzzer.stopBuzzing();
    assert(digitalRead(TEST_BUZZER_PIN) == LOW);

    fakeMillis += 5;
    buzzer.startBuzzing(); // starting again after stop restarts from interval 0
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    std::cout << "test_buzzer_starts_repeats_and_stops passed\n";
}

static void test_buzzer_millis_rollover()
{
    test_double::reset();
    const uint32_t intervals[2] = {50, 30}; // even N, see Buzzer.h
    Buzzer<2> buzzer(TEST_BUZZER_PIN, intervals);
    buzzer.setup();

    fakeMillis = 0xFFFFFFFFu - 10; // start 10ms before millis() wraps to 0
    buzzer.startBuzzing();
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    fakeMillis = 20; // wrapped; 31ms elapsed since start, < 50ms interval 0
    buzzer.startBuzzing();
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    fakeMillis = 40; // 51ms elapsed since start, interval 0 elapsed across the wrap -> toggle
    buzzer.startBuzzing();
    assert(digitalRead(TEST_BUZZER_PIN) == LOW);

    fakeMillis = 70; // 30ms into interval 1 (30ms) -> toggle, wraps back to interval 0, HIGH again
    buzzer.startBuzzing();
    assert(digitalRead(TEST_BUZZER_PIN) == HIGH);

    std::cout << "test_buzzer_millis_rollover passed\n";
}

static void test_switchable_with_delay_blocks_rapid_changes()
{
    test_double::reset();
    SwitchableWithDelay valve(TEST_VALVE_PIN, 1000);
    valve.setup();

    // Starts at t=1, not t=0: lastStateChangeMs==0 doubles as the "never
    // switched" sentinel (see Controller.h), so a change landing exactly at
    // millis()==0 is a known, documented corner case covered separately
    // below, not what this test is exercising.
    fakeMillis = 1;
    valve.switchOn(); // first-ever change is allowed immediately
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    fakeMillis = 501;
    valve.switchOff(); // only 500ms since last change, blocked
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    fakeMillis = 1000;
    valve.switchOff(); // still blocked (999ms elapsed)
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    fakeMillis = 1001;
    valve.switchOff(); // exactly the delay has elapsed, allowed
    assert(digitalRead(TEST_VALVE_PIN) == LOW);

    fakeMillis = 1201;
    valve.switchOn(); // only 200ms since last change, blocked
    assert(digitalRead(TEST_VALVE_PIN) == LOW);

    fakeMillis = 2001;
    valve.switchOn(); // 1000ms elapsed, allowed
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    std::cout << "test_switchable_with_delay_blocks_rapid_changes passed\n";
}

static void test_switchable_with_delay_zero_millis_sentinel_corner_case()
{
    // Documents a known, accepted corner case (see the lastStateChangeMs
    // comment in Controller.h): a state change landing at exactly
    // millis()==0 is indistinguishable from "never switched", so the very
    // next call bypasses the delay instead of enforcing it. millis()
    // essentially never lands back on exactly 0 again for the life of the
    // device, so this is judged not worth the extra state it would take to
    // fix. If this ever needs to change, this test is what to update.
    test_double::reset();
    SwitchableWithDelay valve(TEST_VALVE_PIN, 1000);
    valve.setup();

    fakeMillis = 0;
    valve.switchOn();
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    fakeMillis = 1;    // far less than the 1000ms delay
    valve.switchOff(); // bypassed anyway, due to the sentinel corner case
    assert(digitalRead(TEST_VALVE_PIN) == LOW);

    std::cout << "test_switchable_with_delay_zero_millis_sentinel_corner_case passed\n";
}

static void test_switchable_with_delay_immediate_bypass()
{
    test_double::reset();
    SwitchableWithDelay valve(TEST_VALVE_PIN, 1000);
    valve.setup();

    fakeMillis = 0;
    valve.switchOnNow();
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    fakeMillis = 100;     // well within the delay window
    valve.switchOffNow(); // *Now() bypasses the delay entirely
    assert(digitalRead(TEST_VALVE_PIN) == LOW);

    fakeMillis = 150;
    valve.switchOn(); // ordinary switchOn still respects the delay from switchOffNow()
    assert(digitalRead(TEST_VALVE_PIN) == LOW);

    fakeMillis = 1100;
    valve.switchOn(); // 1000ms elapsed since the switchOffNow(), allowed
    assert(digitalRead(TEST_VALVE_PIN) == HIGH);

    std::cout << "test_switchable_with_delay_immediate_bypass passed\n";
}

static void test_water_level_sensor_semantics()
{
    test_double::reset();
    WaterLevelSensor sensor(TEST_SENSOR_PIN);
    sensor.setup();

    pinState[TEST_SENSOR_PIN] = LOW;
    assert(sensor.isNotSensingWater());

    pinState[TEST_SENSOR_PIN] = HIGH;
    assert(!sensor.isNotSensingWater());

    std::cout << "test_water_level_sensor_semantics passed\n";
}

int main()
{
    test_buzzer_starts_repeats_and_stops();
    test_buzzer_millis_rollover();
    test_switchable_with_delay_blocks_rapid_changes();
    test_switchable_with_delay_zero_millis_sentinel_corner_case();
    test_switchable_with_delay_immediate_bypass();
    test_water_level_sensor_semantics();

    std::cout << "All tests passed\n";
    return 0;
}
