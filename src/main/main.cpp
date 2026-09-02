/**
 * How to upload code to ATtiny
 * https://www.youtube.com/watch?v=hLXDO2sh5XQ&t=200s&ab_channel=Anubits64
 * https://makbit.com/web/firmware/breathing-life-into-digispark-clone-with-attiny-mcu/
 */
#include <Arduino.h>
#include <avr/wdt.h>

#include "Buzzer.h"
#include "Switchable.h"
#include "SwitchableWithDelay.h"
#include "WaterLevelSensor.h"

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
namespace Pins
{
    const uint8_t Reset = 5;             // PB5 / PCINT5 / ^RESET / ADC0 / dW
    const uint8_t ValveSwitch = 3;       // PB3 / PCINT3 / XTAL1 / CLKI / ^OC1B / ADC3
    const uint8_t AlarmLed = 4;          // PB4 / PCINT4 / XTAL2 / CLKO /  OC1B / ADC2
    const uint8_t AlarmBuzzer = 0;       // PB0 / MOSI / DI / SDA / AIN0 / OC0A / ^OC1A / AREF / PCINT0
    const uint8_t HighLevelSensor = 1;   // PB1 / MISO / DO / AIN1 / OC0B / OC1A / PCINT1
    const uint8_t NormalLevelSensor = 2; // PB2 / SCK / USCK / SCL / ADC1 / T0 / INT0 / PCINT2
}

const uint8_t BUZZ_INTERVALS = 6;
const uint32_t buzzIntervalDurations[BUZZ_INTERVALS] = {700, 400, 700, 400, 1400, 4000}; /* beep, pause, beep... */
Buzzer<BUZZ_INTERVALS> alarmBuzzer = Buzzer<BUZZ_INTERVALS>(Pins::AlarmBuzzer, buzzIntervalDurations);

Switchable alarmLed = Switchable(Pins::AlarmLed);

WaterLevelSensor normalLevelSensor = WaterLevelSensor(Pins::NormalLevelSensor);
WaterLevelSensor highLevelSensor = WaterLevelSensor(Pins::HighLevelSensor);

const uint32_t VALVE_DELAY_MS = 16 * 1000ul; /* seconds * milliseconds */
SwitchableWithDelay valve = SwitchableWithDelay(Pins::ValveSwitch, VALVE_DELAY_MS);

void setup()
{
    wdt_disable(); /* clear any watchdog state left over from a prior reset/bootloader before (re)arming it below */

    alarmBuzzer.setup();
    alarmLed.setup();
    normalLevelSensor.setup();
    highLevelSensor.setup();
    valve.setup();

    delay(32);

    /* loop() has no blocking calls and runs continuously, so a hang here is
     * a firmware bug/lockup, not normal operation. 2 seconds gives ample
     * margin above one iteration while still recovering promptly; matches
     * the convention used by the sibling AquariumATO project. Without this,
     * a hang could leave the valve stuck in whatever state it was last
     * driven to, including ON. */
    wdt_enable(WDTO_2S);
}

void loop()
{
    wdt_reset();

    if (highLevelSensor.isNotSensingWater())
    {
        alarmLed.switchOff();
        alarmBuzzer.stopBuzzing();

        if (normalLevelSensor.isNotSensingWater())
        {
            valve.switchOn();
        }
        else
        {
            valve.switchOff();
        }
    }
    else
    {
        valve.switchOffNow();
        alarmLed.switchOn();
        alarmBuzzer.startBuzzing();
    }
}
