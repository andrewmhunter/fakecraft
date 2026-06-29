#ifndef TIMER_HPP
#define TIMER_HPP

typedef struct {
    double remainingTime;
    double startTime;
} Timer;

Timer timerInit(double startTime);
bool timerUpdate(Timer* timer, float deltaTime);
bool timerFinished(Timer* timer);
void timerReset(Timer* timer);
void timerResetTime(Timer* timer, double startTime);

#endif
