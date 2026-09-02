#ifndef RO_WATER_VALVE_CONTROLLER_SWITCHABLE_H
#define RO_WATER_VALVE_CONTROLLER_SWITCHABLE_H

/**
 * A basic ON/OFF digital output.
 *
 * Depends only on pinMode/digitalWrite and the HIGH/LOW/OUTPUT constants,
 * not on <Arduino.h> itself, so it compiles both against the real Arduino
 * core (on-device) and a host test double (see test/ArduinoTestDouble.h).
 */
class Switchable
{
private:
    const uint8_t pin;

    bool isOn = false;

public:
    explicit Switchable(const uint8_t pin) : pin(pin) {}

    virtual ~Switchable() = default;

    void setup()
    {
        pinMode(Switchable::pin, OUTPUT);
        delay(16);
    }

    virtual void switchOn()
    {
        if (!Switchable::isOn)
        {
            digitalWrite(Switchable::pin, HIGH);
            Switchable::isOn = true;
        }
    }

    virtual void switchOff()
    {
        if (Switchable::isOn)
        {
            digitalWrite(Switchable::pin, LOW);
            Switchable::isOn = false;
        }
    }
};

#endif
