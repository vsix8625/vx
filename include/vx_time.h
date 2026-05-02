#ifndef VX_TIME_H_
#define VX_TIME_H_

#include "vx_defs.h"

void vx_init_time(void);

/*
 * Get the number of nanoseconds since vx library initialization
 */
u64 vx_timeNS(void);

/*
 * Get the number of microseconds since vx library initialization
 */
u64 vx_time_micro(void);

/*
 * Get the number of millieseconds since vx library initialization
 */
u64 vx_timeMS(void);

/*
 * Get the number of seconds (`float`) since vx library initialization.
 */
f32 vx_timef(void);

/*
 * Get the number of seconds (`double`) since vx library initialization.
 */
f64 vx_timef64(void);

#endif  // VX_TIME_H_
