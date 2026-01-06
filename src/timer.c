#include <raylib.h>
#include "timer.h"
#include "stdbool.h"

Timer timerInit(double startTime) {
    return (Timer) {
        .remainingTime = startTime,
        .startTime = startTime,
    };
}

bool timerUpdate(Timer* timer) {
    if (timer->remainingTime <= 0) {
        return false;
    }

    timer->remainingTime -= GetFrameTime();

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

