/**
 * How to upload code to ATtiny
 * https://makbit.com/web/firmware/breathing-life-into-digispark-clone-with-attiny-mcu/
 */
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
namespace ArduinoPins {
    const uint8_t Reset = 5;                // PB5 / PCINT5 / ^RESET / ADC0 / dW
    const uint8_t ValveSwitch = 3;          // PB3 / PCINT3 / XTAL1 / CLKI / ^OC1B / ADC3
    const uint8_t AlarmLed = 4;             // PB4 / PCINT4 / XTAL2 / CLKO /  OC1B / ADC2
    const uint8_t AlarmBuzzer = 0;          // PB0 / MOSI / DI / SDA / AIN0 / OC0A / ^OC1A / AREF / PCINT0
    const uint8_t HighLevelSensor = 1;      // PB1 / MISO / DO / AIN1 / OC0B / OC1A / PCINT1
    const uint8_t NormalLevelSensor = 2;    // PB2 / SCK / USCK / SCL / ADC1 / T0 / INT0 / PCINT2
}

class Switchable {
private:
    const uint8_t arduinoPin;
    const uint32_t delayMs;
    bool isOn = false;
public:
    explicit Switchable(const uint8_t arduinoPin) : arduinoPin(arduinoPin), delayMs(0) {
        pinMode(Switchable::arduinoPin, OUTPUT);
        digitalWrite(Switchable::arduinoPin, LOW);
    }

    explicit Switchable(const uint8_t mcuPin, const uint32_t delayMs) : arduinoPin(mcuPin), delayMs(delayMs) {
        pinMode(Switchable::arduinoPin, OUTPUT);
        digitalWrite(Switchable::arduinoPin, LOW);
    }

    virtual ~Switchable() = default;

    void setOn() {
        if (!(Switchable::isOn)) {
            if (delayMs > 0) {
                delay(delayMs);
            }
            Switchable::isOn = true;
            digitalWrite(Switchable::arduinoPin, HIGH);
        }
    }

    void setOff() {
        if (Switchable::isOn) {
            if (delayMs > 0) {
                delay(delayMs);
            }
            Switchable::isOn = false;
            digitalWrite(Switchable::arduinoPin, LOW);
        }
    }
};

template<uint8_t N>
class Buzzer {
private:
    const uint8_t arduinoPin;
    const uint32_t (&intervalDurations)[N];

    uint8_t currentIntervalIndex = 0;
    uint32_t currentIntervalStartMs = 0;

    bool isBuzzing = false;

public:
    explicit Buzzer(const uint8_t arduinoPin, const uint32_t (&intervalDurations)[N]) :
            arduinoPin(arduinoPin),
            intervalDurations(intervalDurations) {

        pinMode(Buzzer::arduinoPin, OUTPUT);
        digitalWrite(Buzzer::arduinoPin, LOW);
    }

    virtual ~Buzzer() = default;

    void setOn() {
        if (!(Buzzer::isBuzzing) && (N > 0)) {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = millis();
            digitalWrite(Buzzer::arduinoPin, HIGH);
        }
    }

    void setOff() {
        if (Buzzer::isBuzzing) {
            Buzzer::isBuzzing = false;
            digitalWrite(Buzzer::arduinoPin, LOW); // immediately stop buzzing
        }
    }

    void loop() {
        if (Buzzer::isBuzzing) {
            if (millis() - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex]) {
                digitalWrite(Buzzer::arduinoPin, !digitalRead(Buzzer::arduinoPin));
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = millis();
            }
        }
    }
};

const uint32_t valveDelayMs = 3 * 60 * 1000; // minutes * seconds * milliseconds
Switchable valveSwitch = Switchable(ArduinoPins::ValveSwitch, valveDelayMs);

/* todo: investigate why the led cannot be used for alarm signalling, it breaks the whole logic */
Switchable redLed = Switchable(ArduinoPins::AlarmLed); // this should have been an alarm led but the code does not work when it's used as one :(

const uint8_t buzzIntervals = 6;
const uint32_t buzzIntervalDurations[buzzIntervals] = {600, 400, 600, 400, 1200, 4000}; // beep, pause, beep, pause, ...
Buzzer<buzzIntervals> alarmBuzzer = Buzzer<buzzIntervals>(ArduinoPins::AlarmBuzzer, buzzIntervalDurations);

void setup() {
    pinMode(ArduinoPins::NormalLevelSensor, INPUT_PULLUP);
    pinMode(ArduinoPins::HighLevelSensor, INPUT_PULLUP);
}

void loop() {
    if (digitalRead(ArduinoPins::HighLevelSensor) == LOW) {
        redLed.setOff();
        alarmBuzzer.setOff();

        if (digitalRead(ArduinoPins::NormalLevelSensor) == LOW) {
            valveSwitch.setOn();
            redLed.setOn(); // <- must be here or the circuit will not work
        } else {
            valveSwitch.setOff();
            redLed.setOff(); // <- must be here or the circuit will not work
        }
    } else {
        valveSwitch.setOff();
//        redLed.setOn(); // <- should have been here, but circuit will not work
        alarmBuzzer.setOn();
        alarmBuzzer.loop();
    }
}
