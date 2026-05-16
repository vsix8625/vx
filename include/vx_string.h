#pragma GCC system_header
#ifndef VX_STRING_H_
    #define VX_STRING_H_

    #include "vx_limits.h"

    #include <string.h>

static inline bool vx_strncmplit(const char *s, size_t len, const char *lit, size_t lit_len)
{
    if (len != lit_len)
    {
        return false;
    }

    return memcmp(s, lit, lit_len) == 0;
}

VX_API i32 vx_pathncmp(const char *s1, const char *s2, size_t n);

VX_API bool vx_path_parent(char *path);

VX_API vx_sv vx_sv_from_cstr(const char *cstr);

/**
 * Compares a vx_sv against a null-terminated C-string.
 * Returns 0 if they are identical in content and length.
 */
VX_API i32 vx_sv_strcmp(vx_sv sv, const char *cstr);

#endif  // VX_STRING_H_
