#include <Arduino.h>

/**
 * <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf">Atmel ATtiny 25/45/85</a>
 * <p>Pin 1 is /RESET</p>
 * \code
 *                  +-\/-+
 * Ain0 (D 5) PB5  1|    |8  Vcc
 * Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
 * Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
 *            GND  4|    |5  PB0 (D 0) pwm0
 *                  +----+
 * \endcode
 */
namespace McuPins {
    constexpr uint8_t Reset = 5;                // PB5 / PCINT5 / ^RESET / ADC0 / dW
    constexpr uint8_t ValveSwitch = 3;          // PB3 / PCINT3 / XTAL1 / CLKI / ^OC1B / ADC3
    constexpr uint8_t AlarmLed = 4;             // PB4 / PCINT4 / XTAL2 / CLKO /  OC1B / ADC2
    constexpr uint8_t AlarmBuzzer = 0;          // PB0 / MOSI / DI / SDA / AIN0 / OC0A / ^OC1A / AREF / PCINT0
    constexpr uint8_t HighLevelSensor = 1;      // PB1 / MISO / DO / AIN1 / OC0B / OC1A / PCINT1
    constexpr uint8_t NormalLevelSensor = 2;    // PB2 / SCK / USCK / SCL / ADC1 / T0 / INT0 / PCINT2
}

class Switchable {
private:
    const uint8_t mcuPin;
    bool isOn = false;
public:
    explicit Switchable(const uint8_t mcuPin) : mcuPin(mcuPin) {
        pinMode(Switchable::mcuPin, OUTPUT);
        digitalWrite(Switchable::mcuPin, LOW);
    }

    virtual ~Switchable() = default;

    void setOn() {
        if (!isOn) {
            Switchable::isOn = true;
            digitalWrite(Switchable::mcuPin, HIGH);
        }
    }

    void setOff() {
        if (isOn) {
            Switchable::isOn = false;
            digitalWrite(Switchable::mcuPin, LOW);
        }
    }
};

template<uint8_t N>
class Buzzer {
private:
    const uint8_t mcuBuzzerPin;
    const uint32_t (&intervalDurations)[N];

    uint8_t currentIntervalIndex = 0;
    uint32_t currentIntervalStartMs = 0;

    bool isBuzzing = false;

public:
    explicit Buzzer(const uint8_t mcuBuzzerPin, const uint32_t (&intervalDurations)[N]) :
            mcuBuzzerPin(mcuBuzzerPin),
            intervalDurations(intervalDurations) {

        pinMode(Buzzer::mcuBuzzerPin, OUTPUT);
        digitalWrite(Buzzer::mcuBuzzerPin, LOW);
    }

    virtual ~Buzzer() = default;

    void setOn() {
        if (!(Buzzer::isBuzzing) && (N > 0)) {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = millis();
            digitalWrite(Buzzer::mcuBuzzerPin, HIGH);
        }
    }

    void setOff() {
        if (Buzzer::isBuzzing) {
            Buzzer::isBuzzing = false;
            digitalWrite(Buzzer::mcuBuzzerPin, LOW); // immediately stop buzzing
        }
    }

    void loop() {
        if (Buzzer::isBuzzing) {
            if (millis() - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex]) {
                digitalWrite(Buzzer::mcuBuzzerPin, !digitalRead(Buzzer::mcuBuzzerPin));
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = millis();
            }
        }
    }
};

Switchable valveSwitch{McuPins::ValveSwitch};
Switchable alarmLed{McuPins::AlarmLed};

constexpr uint32_t buzzIntervalDurations[6] = {700, 400, 700, 400, 1400, 4000}; // beep, pause, beep, pause, ...
Buzzer<(sizeof(buzzIntervalDurations) / sizeof(*buzzIntervalDurations))> alarmBuzzer{McuPins::AlarmBuzzer, buzzIntervalDurations};

void setup() {
    pinMode(McuPins::NormalLevelSensor, INPUT_PULLUP);
    pinMode(McuPins::HighLevelSensor, INPUT_PULLUP);
}

void loop() {
    if (digitalRead(McuPins::HighLevelSensor) == LOW) {
        alarmLed.setOff();
        alarmBuzzer.setOff();

        if (digitalRead(McuPins::NormalLevelSensor) == LOW) {
            valveSwitch.setOn();
        } else {
            valveSwitch.setOff();
        }
    } else {
        valveSwitch.setOff();
        alarmLed.setOn();
        alarmBuzzer.setOn();
        alarmBuzzer.loop();
    }
}
