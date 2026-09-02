#ifndef RO_WATER_VALVE_CONTROLLER_BUZZER_H
#define RO_WATER_VALVE_CONTROLLER_BUZZER_H

/**
 * Repeating alarm pattern (beep/pause/beep/.../long pause, then repeat) on
 * a single digital pin.
 *
 * Depends only on pinMode/digitalWrite/digitalRead/millis/delay and the
 * HIGH/LOW/OUTPUT constants, not on <Arduino.h> itself, so it compiles both
 * against the real Arduino core (on-device) and a host test double (see
 * test/ArduinoTestDouble.h, test/controllerTest.cpp).
 *
 * Exposes both a zero-arg method (reads millis() internally, used by
 * production code) and a uint32_t nowMs overload (pure, deterministic, used
 * by tests to drive time explicitly).
 */
template <uint8_t N>
class Buzzer
{
private:
    const uint8_t pin;
    const uint32_t (&intervalDurations)[N];

    uint8_t currentIntervalIndex = 0;
    uint32_t currentIntervalStartMs = 0;

    bool isBuzzing = false;

public:
    explicit Buzzer(
        const uint8_t pin,
        const uint32_t (&intervalDurations)[N])
        : pin(pin),
          intervalDurations(intervalDurations) {}

    virtual ~Buzzer() = default;

    void setup()
    {
        pinMode(Buzzer::pin, OUTPUT);
        delay(16);
    }

    /**
     * Checks whether the current interval has elapsed as of nowMs and, if
     * so, toggles the pin and advances to the next interval. No-op while
     * not buzzing.
     */
    void process(uint32_t nowMs)
    {
        if (Buzzer::isBuzzing)
        {
            if (nowMs - Buzzer::currentIntervalStartMs >= Buzzer::intervalDurations[Buzzer::currentIntervalIndex])
            {
                digitalWrite(Buzzer::pin, !digitalRead(Buzzer::pin));
                Buzzer::currentIntervalIndex = (Buzzer::currentIntervalIndex + 1) % N;
                Buzzer::currentIntervalStartMs = nowMs;
            }
        }
    }

    void startBuzzing(uint32_t nowMs)
    {
        if (!(Buzzer::isBuzzing) && (N > 0))
        {
            Buzzer::isBuzzing = true;
            Buzzer::currentIntervalIndex = 0;
            Buzzer::currentIntervalStartMs = nowMs;
            digitalWrite(Buzzer::pin, HIGH);
        }

        process(nowMs);
    }

    void startBuzzing()
    {
        startBuzzing(millis());
    }

    void stopBuzzing()
    {
        if (Buzzer::isBuzzing)
        {
            Buzzer::isBuzzing = false;
            digitalWrite(Buzzer::pin, LOW); /* immediately stop buzzing */
        }
    }
};

#endif
