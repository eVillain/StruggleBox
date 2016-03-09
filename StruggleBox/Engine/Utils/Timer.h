#ifndef TIMER_H
#define TIMER_H

#include <string>

/// Universal timers
class Timer
{
public:
    static long Microseconds();
    static double Milliseconds();
    static double Seconds();
    static std::string TimeStamp();
};

#endif /* TIMER_H */
