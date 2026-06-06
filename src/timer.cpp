#include <stdbool.h>
#include <raylib.h>
#include "timer.hpp"

Timer timerInit(double startTime) {
    return (Timer) {
        .remainingTime = startTime,
        .startTime = startTime,
    };
}

bool timerUpdate(Timer* timer, float deltaTime) {
    if (timer->remainingTime <= 0) {
        return false;
    }

    timer->remainingTime -= deltaTime;

    return timer->remainingTime <= 0;
}

bool timerFinished(Timer* timer) {
    return timer->remainingTime <= 0;
}

void timerReset(Timer* timer) {
    timer->remainingTime = timer->startTime;
}

void timerResetTime(Timer* timer, double startTime) {
    timer->startTime = startTime;
    timerReset(timer);
}

