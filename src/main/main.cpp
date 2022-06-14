/**
 * How to upload code to ATtiny
 * https://www.youtube.com/watch?v=hLXDO2sh5XQ&t=200s&ab_channel=Anubits64
 * https://makbit.com/web/firmware/breathing-life-into-digispark-clone-with-attiny-mcu/
 */
#include <Arduino.h>

/* Classes - Start */
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
            digitalWrite(Buzzer::pin, LOW); /* immediately stop buzzing */
        }
    }
};

class Switchable {
private:
    const uint8_t pin;

    bool isOn = false;

public:
    explicit Switchable(const uint8_t pin) : pin(pin) {}

    virtual ~Switchable() = default;

    void setup() {
        pinMode(Switchable::pin, OUTPUT);
        delay(16);
    }

    virtual void switchOn() {
        if (!Switchable::isOn) {
            digitalWrite(Switchable::pin, HIGH);
            Switchable::isOn = true;
        }
    }

    virtual void switchOff() {
        if (Switchable::isOn) {
            digitalWrite(Switchable::pin, LOW);
            Switchable::isOn = false;
        }
    }
};

class SwitchableWithDelay {
private:
    const uint8_t pin;
    const uint32_t delayMs;

    bool isOn = false;
    uint32_t lastStateChangeMs = 0ul;

public:
    explicit SwitchableWithDelay(const uint8_t pin, const uint32_t delayMs) : pin(pin), delayMs(delayMs) {}

    virtual ~SwitchableWithDelay() = default;

    void setup() {
        pinMode(SwitchableWithDelay::pin, OUTPUT);
        delay(16);
    }

    void switchOnNow() {
        digitalWrite(SwitchableWithDelay::pin, HIGH);
        isOn = true;
        lastStateChangeMs = millis();
    }

    void switchOffNow() {
        digitalWrite(SwitchableWithDelay::pin, LOW);
        isOn = false;
        lastStateChangeMs = millis();
    }

    void switchOn() {
        if (!isOn) {
            if (lastStateChangeMs == 0ul || millis() - lastStateChangeMs >= delayMs) {
                switchOnNow();
            }
        }
    }

    void switchOff() {
        if (isOn) {
            if (lastStateChangeMs == 0ul || millis() - lastStateChangeMs >= delayMs) {
                switchOffNow();
            }
        }
    }
};

class WaterLevelSensor {
private:
    const uint8_t pin;

public:
    explicit WaterLevelSensor(const uint8_t pin) : pin(pin) {}

    virtual ~WaterLevelSensor() = default;

    void setup() {
        pinMode(WaterLevelSensor::pin, INPUT_PULLUP);
        delay(16);
    }

    bool isSensingWater() {
        return digitalRead(WaterLevelSensor::pin) == HIGH;
    }

    bool isNotSensingWater() {
        return digitalRead(WaterLevelSensor::pin) == LOW;
    }
};
/* Classes - End */

/**
 * <a href="http://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf">Atmel ATtiny 25/45/85</a>
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

const uint8_t BUZZ_INTERVALS = 6;
const uint32_t buzzIntervalDurations[BUZZ_INTERVALS] = {700, 400, 700, 400, 1400, 4000}; /* beep, pause, beep... */
Buzzer<BUZZ_INTERVALS> alarmBuzzer = Buzzer<BUZZ_INTERVALS>(Pins::AlarmBuzzer, buzzIntervalDurations);

Switchable alarmLed = Switchable(Pins::AlarmLed);

WaterLevelSensor normalLevelSensor = WaterLevelSensor(Pins::NormalLevelSensor);
WaterLevelSensor highLevelSensor = WaterLevelSensor(Pins::HighLevelSensor);

const uint32_t VALVE_DELAY_MS = 16 * 1000ul; /* seconds * milliseconds */
SwitchableWithDelay valve = SwitchableWithDelay(Pins::ValveSwitch, VALVE_DELAY_MS);

void setup() {
    alarmBuzzer.setup();
    alarmLed.setup();
    normalLevelSensor.setup();
    highLevelSensor.setup();
    valve.setup();

    delay(32);
}

void loop() {
    if (highLevelSensor.isNotSensingWater()) {
        alarmLed.switchOff();
        alarmBuzzer.stopBuzzing();

        if (normalLevelSensor.isNotSensingWater()) {
            valve.switchOn();
        } else {
            valve.switchOff();
        }
    } else {
        valve.switchOffNow();
        alarmLed.switchOn();
        alarmBuzzer.startBuzzing();
    }
}
