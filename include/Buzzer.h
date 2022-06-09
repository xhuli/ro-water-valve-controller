#ifndef ROWATERCONTROLLER_BUZZER_H
#define ROWATERCONTROLLER_BUZZER_H
#pragma once

template<uint8_t N>
class Buzzer {
private:
    const uint8_t pin;
    const uint32_t (&intervalDurations)[N];

    uint8_t currentIntervalIndex = 0;
    uint32_t currentIntervalStartMs = 0;

    bool isBuzzing = false;

public:
    explicit Buzzer(const uint8_t pin, const uint32_t (&intervalDurations)[N]) :
            pin(pin),
            intervalDurations(intervalDurations) {}

    virtual ~Buzzer() = default;

    void setup() {
        pinMode(Buzzer::pin, OUTPUT);
        delay(16);
    }

    void startBuzzing() {
        if (!(Buzzer::isBuzzing) && (N > 0)) {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = millis();
            digitalWrite(Buzzer::pin, HIGH);
        }

        if (Buzzer::isBuzzing) {
            if (millis() - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex]) {
                digitalWrite(Buzzer::pin, !digitalRead(Buzzer::pin));
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = millis();
            }
        }
    }

    void stopBuzzing() {
        if (Buzzer::isBuzzing) {
            Buzzer::isBuzzing = false;
            digitalWrite(Buzzer::pin, LOW); // immediately stop buzzing
        }
    }
};

#endif
