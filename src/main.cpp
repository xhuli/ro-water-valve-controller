/**
 * How to upload code to ATtiny
 * https://makbit.com/web/firmware/breathing-life-into-digispark-clone-with-attiny-mcu/
 */
#include <Arduino.h>

/**
 * <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf">Atmel ATtiny 25/45/85</a>
 * <p>Dip pin 1 is RESET</p>
 * \code
 *                  +-\/-+
 * Ain0 (D 5) PB5  1|    |8  Vcc
 * Ain3 (D 3) PB3  2|    |7  PB2 (D 2) Ain1
 * Ain2 (D 4) PB4  3|    |6  PB1 (D 1) pwm1
 *            GND  4|    |5  PB0 (D 0) pwm0
 *                  +----+
 * \endcode
 */
namespace Pins {
    const uint8_t Reset = 5;                // PB5 / PCINT5 / ^RESET / ADC0 / dW
    const uint8_t ValveSwitch = 3;          // PB3 / PCINT3 / XTAL1 / CLKI / ^OC1B / ADC3
    const uint8_t AlarmLed = 4;             // PB4 / PCINT4 / XTAL2 / CLKO /  OC1B / ADC2
    const uint8_t AlarmBuzzer = 0;          // PB0 / MOSI / DI / SDA / AIN0 / OC0A / ^OC1A / AREF / PCINT0
    const uint8_t HighLevelSensor = 1;      // PB1 / MISO / DO / AIN1 / OC0B / OC1A / PCINT1
    const uint8_t NormalLevelSensor = 2;    // PB2 / SCK / USCK / SCL / ADC1 / T0 / INT0 / PCINT2
}

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

    void setOn() {
        if (!(Buzzer::isBuzzing) && (N > 0)) {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = millis();
            digitalWrite(Buzzer::pin, HIGH);
        }
    }

    void setOff() {
        if (Buzzer::isBuzzing) {
            Buzzer::isBuzzing = false;
            digitalWrite(Buzzer::pin, LOW); // immediately stop buzzing
        }
    }

    void setup() {
        pinMode(Buzzer::pin, OUTPUT);
    }

    void loop() {
        if (Buzzer::isBuzzing) {
            if (millis() - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex]) {
                digitalWrite(Buzzer::pin, !digitalRead(Buzzer::pin));
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = millis();
            }
        }
    }
};

const uint8_t buzzIntervals = 6;
const uint32_t buzzIntervalDurations[buzzIntervals] = {600, 400, 600, 400, 1200, 4000}; // beep, pause, beep, pause, ...
Buzzer<buzzIntervals> alarmBuzzer = Buzzer<buzzIntervals>(Pins::AlarmBuzzer, buzzIntervalDurations);

void setup() {
    pinMode(Pins::AlarmBuzzer, OUTPUT);
    delay(16);

    pinMode(Pins::AlarmLed, OUTPUT);
    delay(16);

    pinMode(Pins::ValveSwitch, OUTPUT);
    delay(16);

    pinMode(Pins::NormalLevelSensor, INPUT_PULLUP);
    delay(16);

    pinMode(Pins::HighLevelSensor, INPUT_PULLUP);

    delay(32);
}

void loop() {
    if (digitalRead(Pins::HighLevelSensor) == LOW) {
        digitalWrite(Pins::AlarmLed, LOW);
        alarmBuzzer.setOff();

        if (digitalRead(Pins::NormalLevelSensor) == LOW) {
            digitalWrite(Pins::ValveSwitch, HIGH);
        } else {
            digitalWrite(Pins::ValveSwitch, LOW);
        }
    } else {
        digitalWrite(Pins::ValveSwitch, LOW);
        digitalWrite(Pins::AlarmLed, HIGH);
        alarmBuzzer.setOn();
        alarmBuzzer.loop();
    }
}
