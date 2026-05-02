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

/*
 * Converts the duration between the start and end ticks into a human-readable string.
 * Returns a pointer to the provided buffer.
 */
char *vx_ticks_format(const vx_ticks *ticks, char *buf, size_t buf_size);

/*
 * Captures high-resolution timestamp and store it as the starting point in a `vx_ticks` structure.
 * Resets the `end` value to 0.
 */
void vx_ticks_start(vx_ticks *t);

/*
 * Captures high-resolution timestamp and store it as the end point in a `vx_ticks` structure.
 * When paired with a preceding `vx_ticks_start` call, this defines the duration of the profiled
 * code block.
 */
void vx_ticks_end(vx_ticks *t);

#endif  // VX_TIME_H_
