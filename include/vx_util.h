#ifndef VX_UTIL_H_
#define VX_UTIL_H_

#include "vx_io.h"

/* Recursively create directories for `path` (like mkdir -p).
 * Returns VX_OK on success, VX_ERROR if creation fails or path is invalid. */
vx_status vx_mkdir_p(const char *path);

char *vx_trim_s(char *s);

#endif  // VX_UTIL_H_
