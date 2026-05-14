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

/*
 * Checks whether the target path is protected from deletion.
 * A path is protected if it is equal to, or a parent of, any forbidden path.
 * This ensures that deleting a protected path or any of its ancestors is blocked,
 * while children of a protected path remain deletable.
 *
 * @param target the resolved absolute path to check.
 * @return: `true` if the path is protected, `false` otherwise.
 */
VX_API bool vx_fs_is_path_protected(const char *target);

VX_API void vx_fs_log_forbidden_paths(void);

VX_API bool vx_path_parent(char *path);

VX_API vx_sv vx_sv_from_cstr(const char *cstr);

/**
 * Compares a vx_sv against a null-terminated C-string.
 * Returns 0 if they are identical in content and length.
 */
VX_API i32 vx_sv_strcmp(vx_sv sv, const char *cstr);

#endif  // VX_STRING_H_
