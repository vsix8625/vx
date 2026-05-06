#ifndef VX_FS_H_
#define VX_FS_H_

#include "vx_platform.h"

/*
 * Move/Rename `src` to `dest`.
 * Returns `true` on success, or `false` on failure.
 */
VX_API bool vx_fs_mv(const char *src, const char *dest);

/*
 * Uses `realpath` from `<stdlib.h>` in `Linux` and `_wfullpath` in `Windows`.
 *
 * NOTE: If resolved is `nullptr` it will return a heap allocated buffer, caller is responsible
 * to free().
 */
VX_API char *vx_fs_realpath(const char *path, char *resolved);

VX_API bool vx_fs_rmrf(const char *path);

#endif  // VX_FS_H_
