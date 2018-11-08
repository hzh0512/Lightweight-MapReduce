#ifndef LMR_MAIN_COMMON_H
#define LMR_MAIN_COMMON_H

#include <ctime>

inline void sleep_us(int useconds)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = useconds;
    nanosleep(&ts, nullptr);
}

#endif //LMR_MAIN_COMMON_H
