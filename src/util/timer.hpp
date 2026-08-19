#ifndef TIMER_HPP
#define TIMER_HPP

struct Timer {
    double remainingTime;
    double startTime;

    Timer(double startTime = 0.0);
    bool update(float deltaTime);
    bool finished() const;
    void reset();
    void reset(double startTime);
};

#endif
