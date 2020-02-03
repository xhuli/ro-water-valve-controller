#include <iostream>
#include <cstdint>
#include "MockCommon.h"

template<uint8_t N>
class Buzzer {
private:
    const uint32_t (&intervalDurations)[N];

    uint8_t currentIntervalIndex = 0;
    uint32_t currentIntervalStartMs = 0;

    bool isBuzzing = false;

public:
    explicit Buzzer(const uint32_t (&intervals)[N]) : intervalDurations(intervals) {}

    virtual ~Buzzer() = default;

    void setOn() {
        if (!(Buzzer::isBuzzing) && (N > 0)) {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = millis();

            std::cout << "Buzzer started\n";
        }
    }

    void setOff() {
        if (Buzzer::isBuzzing) {
            Buzzer::isBuzzing = false;

            std::cout << "Buzzer stopped\n";
        }
    }

    void loop() {
        if (Buzzer::isBuzzing) {
            uint32_t currentMillis = millis();
            if (millis() - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex]) {
                uint16_t outValue = (Buzzer::currentIntervalIndex % 2) == 0 ? 0x1 : 0x0;
                std::cout << "currentIntervalIndex = " << static_cast<uint16_t>(Buzzer::currentIntervalIndex) << "\n";
                std::cout << "\toutValue = " << outValue << "\n";
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = millis();
            }
        }
    }
};

int main() {

    constexpr uint32_t buzzIntervals[6] = {700, 400, 700, 400, 1400, 4000};
    Buzzer<(sizeof(buzzIntervals) / sizeof(*buzzIntervals))> alarmBuzzer{buzzIntervals};

    alarmBuzzer.setOn();

    uint32_t startMillis = millis();

    while(millis() < (startMillis + 30000)) {
        alarmBuzzer.loop();

        timeKeeper.loop();
    }

    alarmBuzzer.setOff();

    return 0;
}
