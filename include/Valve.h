#ifndef ROWATERCONTROLLER_VALVE_H
#define ROWATERCONTROLLER_VALVE_H
#pragma once

class Valve {
private:
    const uint8_t pin;
    const uint32_t delayMs;

    uint32_t lastStateChangeMs = 0;
    bool isOpen = false;

public:
    explicit Valve(const uint8_t pin, const uint32_t delayMs) :
            pin(pin),
            delayMs(delayMs) {}

    virtual ~Valve() = default;

    void setup() {
        pinMode(pin, OUTPUT);
        delay(16);
    }

    void openValve() {
        if (!Valve::isOpen) {
            if (millis() - Valve::lastStateChangeMs >= Valve::delayMs) {
                digitalWrite(Valve::pin, HIGH);
                Valve::lastStateChangeMs = millis();
                Valve::isOpen = true;
            }
        }
    }

    void closeValve(){
        if (Valve::isOpen) {
            if (millis() - Valve::lastStateChangeMs >= Valve::delayMs) {
                digitalWrite(Valve::pin, LOW);
                Valve::lastStateChangeMs = millis();
                Valve::isOpen = false;
            }
        }
    }
};

#endif
