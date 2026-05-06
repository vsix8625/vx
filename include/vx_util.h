#ifndef VX_UTIL_H_
#define VX_UTIL_H_

#include "vx_io.h"

#define VX_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define VX_CAST(type, v)   ((type) v)

/* Recursively create directories for `path` (like mkdir -p).
 * Returns VX_OK on success, VX_ERROR if creation fails or path is invalid. */
VX_API vx_status vx_mkdir_p(const char *path);

VX_API char *vx_trim_s(char *s);

VX_API void vx_yield(void);

#endif  // VX_UTIL_H_
