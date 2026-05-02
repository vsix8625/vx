#ifndef VX_TIME_H_
#define VX_TIME_H_

#include "vx_platform.h"
#include "vx_defs.h"

VX_API void vx_init_time(void);

/*
 * Get the number of nanoseconds since vx library initialization
 */
VX_API u64 vx_time_ns(void);

/*
 * Get the number of microseconds since vx library initialization
 */
VX_API u64 vx_time_micro(void);

/*
 * Get the number of millieseconds since vx library initialization
 */
VX_API u64 vx_time_ms(void);

/*
 * Get the number of seconds (`float`) since vx library initialization.
 */
VX_API f32 vx_time_f32(void);

/*
 * Get the number of seconds (`double`) since vx library initialization.
 */
VX_API f64 vx_time_f64(void);

/*
 * Converts the duration between the start and end ticks into a human-readable string.
 * Returns a pointer to the provided buffer.
 */
VX_API char *vx_ticks_format(const vx_ticks *ticks, char *buf, size_t buf_size);

/*
 * Captures high-resolution timestamp and store it as the starting point in a `vx_ticks` structure.
 * Resets the `end` value to 0.
 */
VX_API void vx_ticks_start(vx_ticks *t);

/*
 * Captures high-resolution timestamp and store it as the end point in a `vx_ticks` structure.
 * When paired with a preceding `vx_ticks_start` call, this defines the duration of the profiled
 * code block.
 */
VX_API void vx_ticks_end(vx_ticks *t);

#endif  // VX_TIME_H_
