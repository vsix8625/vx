#pragma GCC system_header
#ifndef VX_STRING_H_
    #define VX_STRING_H_

    #include "vx_platform.h"
    #include <string.h>
    #include "vx_defs.h"

static inline bool vx_strncmplit(const char *s, size_t len, const char *lit, size_t lit_len)
{
    if (len != lit_len)
    {
        return false;
    }

    return memcmp(s, lit, lit_len) == 0;
}

VX_API i32 vx_pathncmp(const char *s1, const char *s2, size_t n);

VX_API void vx_fs_forbid_path(const char *path);
VX_API bool vx_fs_is_path_protected(const char *target);

#endif  // VX_STRING_H_
