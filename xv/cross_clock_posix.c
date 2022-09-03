#if defined(__unix__) || defined(__APPLE__) // && !defined(__MACH__)
#include <stddef.h>
#include <sys/time.h>
#include "cross_clock.h"

static struct timeval start_time = { 0 };
static unsigned long frequency = 0;

void init_cross_clock(void)
{
    gettimeofday(&start_time, NULL);
    frequency = 1000000;
}

unsigned long cross_clock_frequency(void)
{
    return frequency;
}

unsigned long cross_clock_value(void)
{
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return (cur_time.tv_usec - start_time.tv_usec) + ((cur_time.tv_sec - start_time.tv_sec) * frequency);
}

float cross_clock_value_seconds(void)
{
    return (float)cross_clock_value() / (float)frequency;
}
#endif
