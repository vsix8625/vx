#ifndef VX_FS_H_
#define VX_FS_H_

#include "vx_platform.h"

/*
 * Move/Rename `src` to `dest`.
 * Returns `true` on success, or `false` on failure.
 */
VX_API bool vx_fs_mv(const char *src, const char *dest);

#endif  // VX_FS_H_
