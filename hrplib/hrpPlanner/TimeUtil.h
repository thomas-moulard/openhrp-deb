#ifndef __TIME_UTIL_H__
#define __TIME_UTIL_H__

#ifndef tick_t
#ifdef _WIN32
#include <windows.h>
typedef LONGLONG tick_t;
#else
typedef unsigned long long tick_t;
#endif
#endif
#include "exportdef.h"

tick_t HRPPLANNER_API get_tick();
double HRPPLANNER_API get_cpu_frequency();
#define tick2usec(t)	((t)*1e6/get_cpu_frequency())
#define tick2msec(t)	((t)*1e3/get_cpu_frequency())
#define tick2sec(t)	((t)/get_cpu_frequency())

#endif
