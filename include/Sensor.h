#ifndef ROWATERCONTROLLER_SENSOR_H
#define ROWATERCONTROLLER_SENSOR_H
#pragma once

class Sensor {
private:
    const uint8_t pin;

public:
    explicit Sensor(const uint8_t pin) : pin(pin) {}

    virtual ~Sensor() = default;

    void setup() {
        pinMode(Sensor::pin, INPUT_PULLUP);
        delay(16);
    }

    bool isSensingWater() {
        return digitalRead(Sensor::pin) == HIGH;
    }

    bool isNotSensingWater() {
        return digitalRead(Sensor::pin) == LOW;
    }
};

#endif
