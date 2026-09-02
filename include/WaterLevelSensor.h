#ifndef RO_WATER_VALVE_CONTROLLER_WATER_LEVEL_SENSOR_H
#define RO_WATER_VALVE_CONTROLLER_WATER_LEVEL_SENSOR_H

/**
 * A water level sensor input, wired INPUT_PULLUP: HIGH means water is
 * sensed, LOW means it isn't (see claude_custom_instructions.md §7 for the
 * PCB-level rationale - do not invert without verifying the sensor
 * interface).
 *
 * Depends only on pinMode/digitalRead/delay and the INPUT_PULLUP/LOW
 * constants, not on <Arduino.h> itself, so it compiles both against the
 * real Arduino core (on-device) and a host test double (see
 * test/ArduinoTestDouble.h).
 */
class WaterLevelSensor
{
private:
    const uint8_t pin;

public:
    explicit WaterLevelSensor(const uint8_t pin) : pin(pin) {}

    virtual ~WaterLevelSensor() = default;

    void setup()
    {
        pinMode(WaterLevelSensor::pin, INPUT_PULLUP);
        delay(16);
    }

    bool isNotSensingWater()
    {
        return digitalRead(WaterLevelSensor::pin) == LOW;
    }
};

#endif
