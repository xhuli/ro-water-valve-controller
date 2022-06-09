#ifndef ROWATERCONTROLLER_ALARMLED_H
#define ROWATERCONTROLLER_ALARMLED_H
#pragma once

class AlarmLed {
private:
    uint8_t pin;
    bool isLit = false;

public:
    explicit AlarmLed(const uint8_t pin) : pin(pin) {}

    virtual ~AlarmLed() = default;

    void setup() {
        pinMode(AlarmLed::pin, OUTPUT);
        delay(16);
    }

    void turnOn() {
        if (!AlarmLed::isLit) {
            digitalWrite(AlarmLed::pin, HIGH);
            AlarmLed::isLit = true;
        }
    }

    void turnOff() {
        if (AlarmLed::isLit) {
            digitalWrite(AlarmLed::pin, LOW);
            AlarmLed::isLit = false;
        }
    }
};

#endif
