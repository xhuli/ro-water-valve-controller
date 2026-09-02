#ifndef RO_WATER_VALVE_CONTROLLER_SWITCHABLE_WITH_DELAY_H
#define RO_WATER_VALVE_CONTROLLER_SWITCHABLE_WITH_DELAY_H

/**
 * An ON/OFF digital output with an anti-chatter delay between ordinary
 * state changes, plus an immediate *Now() path that bypasses the delay
 * entirely (used for safety-critical shutdowns).
 *
 * Depends only on pinMode/digitalWrite/millis/delay and the HIGH/LOW/OUTPUT
 * constants, not on <Arduino.h> itself, so it compiles both against the
 * real Arduino core (on-device) and a host test double (see
 * test/ArduinoTestDouble.h, test/controllerTest.cpp).
 *
 * Exposes both zero-arg methods (read millis() internally, used by
 * production code) and uint32_t nowMs overloads (pure, deterministic, used
 * by tests to drive time explicitly).
 */
class SwitchableWithDelay
{
private:
    const uint8_t pin;
    const uint32_t delayMs;

    bool isOn = false;
    /* 0ul doubles as "never changed" (see canSwitch()) as well as a real
     * elapsed-time value; millis() essentially never lands back on exactly
     * 0 for the life of this device, so the ambiguity is intentional and
     * safe to leave rather than "fix" into something less simple. */
    uint32_t lastStateChangeMs = 0ul;

    /* True if enough time has passed since the last state change (or there
     * has never been one) to allow another one now. */
    bool canSwitch(uint32_t nowMs) const
    {
        return lastStateChangeMs == 0ul || nowMs - lastStateChangeMs >= delayMs;
    }

public:
    explicit SwitchableWithDelay(
        const uint8_t pin,
        const uint32_t delayMs)
        : pin(pin),
          delayMs(delayMs) {}

    virtual ~SwitchableWithDelay() = default;

    void setup()
    {
        pinMode(SwitchableWithDelay::pin, OUTPUT);
        delay(16);
    }

    void switchOnNow(uint32_t nowMs)
    {
        digitalWrite(SwitchableWithDelay::pin, HIGH);
        isOn = true;
        lastStateChangeMs = nowMs;
    }

    void switchOnNow()
    {
        switchOnNow(millis());
    }

    void switchOffNow(uint32_t nowMs)
    {
        digitalWrite(SwitchableWithDelay::pin, LOW);
        isOn = false;
        lastStateChangeMs = nowMs;
    }

    void switchOffNow()
    {
        switchOffNow(millis());
    }

    void switchOn(uint32_t nowMs)
    {
        if (!isOn && canSwitch(nowMs))
        {
            switchOnNow(nowMs);
        }
    }

    void switchOn()
    {
        switchOn(millis());
    }

    void switchOff(uint32_t nowMs)
    {
        if (isOn && canSwitch(nowMs))
        {
            switchOffNow(nowMs);
        }
    }

    void switchOff()
    {
        switchOff(millis());
    }
};

#endif
