#ifndef _AQUARIUM_CONTROLLER_MOCK_COMMON_H_
#define _AQUARIUM_CONTROLLER_MOCK_COMMON_H_
#pragma once

#include <stdint.h>
#include <random>
#include <time.h>
#include <functional>


uint32_t getRandomUint32() {
    return rand();
};

time_t currentTime = 0;

time_t now() {
    return currentTime;
};

void delay(uint32_t) {
    return;
}

class TimeKeeper {
private:
    uint8_t weekDay;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint32_t millis;
public:

    TimeKeeper() {}

    TimeKeeper(
            uint8_t weekDay,
            uint8_t hour,
            uint8_t minute,
            uint8_t second,
            uint32_t milliSecond
    ) :
            weekDay(weekDay),
            hour(hour),
            minute(minute),
            second(second),
            millis(milliSecond) {}

    uint8_t getWeekDay() const {
        return weekDay;
    }

    void setWeekDay(uint8_t weekDay) {
        TimeKeeper::weekDay = weekDay;
    }

    uint8_t getHour() const {
        return hour;
    }

    void setHour(uint8_t hour) {
        TimeKeeper::hour = hour;
    }

    uint8_t getMinute() const {
        return minute;
    }

    void setMinute(uint8_t minute) {
        TimeKeeper::minute = minute;
    }

    uint8_t getSecond() const {
        return second;
    }

    void setSecond(uint8_t second) {
        TimeKeeper::second = second;
    }

    uint32_t getMillis() const {
        return millis;
    }

    void setMillis(uint32_t millis) {
        TimeKeeper::millis = millis;
    }

    void setup() {
        ++millis;
    }

    void loop() {
        ++millis;
        if (millis % 1000 == 0) {
            ++second;
            if (second % 60 == 0) {
                second = 0;
                ++minute;
                if (minute % 60 == 0) {
                    minute = 0;
                    ++hour;
                    if (hour % 24 == 0) {
                        hour = 0;
                        ++weekDay;
                        if (weekDay % 8 == 0) {
                            weekDay = 1;
                        }
                    }
                }
            }
        }
    }
};

TimeKeeper timeKeeper{1, 0, 0, 0, getRandomUint32()};


uint32_t millis() {
    return timeKeeper.getMillis();
};

uint8_t second() {
    return timeKeeper.getSecond();
};

uint8_t minute() {
    return timeKeeper.getMinute();
};

uint8_t hour() {
    return timeKeeper.getHour();
};

uint8_t weekday() {
    return timeKeeper.getWeekDay();
};


static const char *getDayOfWeek(uint8_t dayOfWeek) {
    switch (dayOfWeek) {
        case 0:
            return "Any";
        case 1:
            return "Sunday";
        case 2:
            return "Monday";
        case 3:
            return "Tuesday";
        case 4:
            return "Wednesday";
        case 5:
            return "Thursday";
        case 6:
            return "Friday";
        case 7:
            return "Saturday";

        default:
            return "Unknown";
    }
};

#endif
