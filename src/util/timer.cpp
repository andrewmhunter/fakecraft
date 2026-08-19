#include <stdbool.h>
#include "timer.hpp"

Timer::Timer(double startTime) : remainingTime{startTime}, startTime{startTime}
{}

bool Timer::update(float deltaTime) {
    if (remainingTime <= 0) {
        return false;
    }

    remainingTime -= deltaTime;

    return remainingTime <= 0;
}

bool Timer::finished() const {
    return remainingTime <= 0;
}

void Timer::reset() {
    remainingTime = startTime;
}

void Timer::reset(double startTime) {
    this->startTime = startTime;
    reset();
}

