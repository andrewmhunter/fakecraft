#ifndef TIMER_H
#define TIMER_H

#include <stdbool.h>

typedef struct {
    double remainingTime;
    double startTime;
} Timer;

Timer timerInit(double startTime);
bool timerUpdate(Timer* timer);
bool timerFinished(Timer* timer);
void timerReset(Timer* timer);
void timerResetTime(Timer* timer, double startTime);

#endif

