/**
 * How to upload code to ATtiny
 * https://makbit.com/web/firmware/breathing-life-into-digispark-clone-with-attiny-mcu/
 */
#include <Arduino.h>
#include "AlarmLed.h"
#include "Buzzer.h"
#include "Sensor.h"
#include "Valve.h"
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

const uint8_t BUZZ_INTERVALS = 6;
const uint32_t buzzIntervalDurations[BUZZ_INTERVALS] = {600, 400, 600, 400, 1200, 4000}; /* beep, pause, beep... */
Buzzer<BUZZ_INTERVALS> alarmBuzzer = Buzzer<BUZZ_INTERVALS>(Pins::AlarmBuzzer, buzzIntervalDurations);

AlarmLed alarmLed = AlarmLed(Pins::AlarmLed);

Sensor normalLevelSensor = Sensor(Pins::NormalLevelSensor);
Sensor highLevelSensor = Sensor(Pins::HighLevelSensor);

const uint32_t VALVE_DELAY_MS = 1 * 60 * 1000; /* minutes x seconds x milliseconds */
Valve valve = Valve(Pins::ValveSwitch, VALVE_DELAY_MS);

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
        alarmLed.turnOff();
        alarmBuzzer.stopBuzzing();

        if (normalLevelSensor.isNotSensingWater()) {
            valve.openValve();
        } else {
            valve.closeValve();
        }
    } else {
        valve.closeValve();
        alarmLed.turnOn();
        alarmBuzzer.startBuzzing();
    }
}
