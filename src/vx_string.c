#include "vx_fs.h"
#include "vx_string.h"
#include "vx_util.h"
#include "vx_io.h"

i32 vx_pathncmp(const char *s1, const char *s2, size_t n)
{
#if defined(VX_OS_WINDOWS)
    return VX_CAST(i32, _strnicmp(s1, s2, n));
#elif defined(VX_OS_LINUX)
    return VX_CAST(i32, strncmp(s1, s2, n));
#endif
}

//----------------------------------------------------------------------------------------------------
// vx_fs helper

#define VX_FS_FORBIDDEN_PATH_MAX_COUNT 32
static char g_vx_fs_forbidden_paths[VX_FS_FORBIDDEN_PATH_MAX_COUNT][VX_PATH_MAX];
static u32  g_vx_fs_forbidden_count = 0;

void vx_fs_forbid_path(const char *path)
{
    if (path == nullptr || path[0] == '\0' ||
        g_vx_fs_forbidden_count >= VX_FS_FORBIDDEN_PATH_MAX_COUNT)
    {
        return;
    }

    char resolved[VX_PATH_MAX];

    if (vx_fs_realpath(path, resolved) == nullptr)
    {
        return;
    }

    for (u32 i = 0; i < g_vx_fs_forbidden_count; i++)
    {
        if (vx_pathncmp(resolved, g_vx_fs_forbidden_paths[i], VX_PATH_MAX) == 0)
        {
            vx_errlog("Path already in forbidden list: %s", resolved);
            return;
        }
    }

    snprintf(g_vx_fs_forbidden_paths[g_vx_fs_forbidden_count], VX_PATH_MAX, "%s", resolved);

    g_vx_fs_forbidden_paths[g_vx_fs_forbidden_count][VX_PATH_MAX - 1] = '\0';

    g_vx_fs_forbidden_count++;
}

bool vx_fs_is_path_protected(const char *target)
{
    if (target == nullptr)
    {
        return true;
    }

    size_t t_len = strlen(target);

    for (u32 i = 0; i < g_vx_fs_forbidden_count; i++)
    {
        const char *restricted = g_vx_fs_forbidden_paths[i];

        size_t r_len = strlen(restricted);

        /* Block if target == restricted (exact) or target is a parent of restricted */
        if (vx_pathncmp(restricted, target, t_len) == 0)
        {
            if (t_len == r_len || restricted[t_len] == VX_PATH_SEP)
            {
                return true;
            }
        }
    }

#if defined(VX_OS_WINDOWS)
    const char *defaults[] = {"C:\\Windows", "C:\\Users", nullptr};
#else
    const char *defaults[] = {"/", "/etc", "/usr", "/bin", "/boot", nullptr};
#endif

    for (u32 i = 0; defaults[i]; ++i)
    {
        const char *restricted = defaults[i];

        size_t r_len = strlen(restricted);

        if (vx_pathncmp(target, restricted, t_len) == 0)
        {
            if (t_len == r_len || restricted[t_len] == VX_PATH_SEP)
            {
                return true;
            }
        }
    }

    return false;
}

void vx_fs_log_forbidden_paths(void)
{
    vx_log("Forbidden paths (%u):", g_vx_fs_forbidden_count);

    for (u32 i = 0; i < g_vx_fs_forbidden_count; i++)
    {
        vx_log("  [%u] %s", i, g_vx_fs_forbidden_paths[i]);
    }
}

//----------------------------------------------------------------------------------------------------

bool vx_path_parent(char *path)
{
    if (path == nullptr || path[0] == '\0')
    {
        return false;
    }

    size_t len = strlen(path);

    while (len > 0 && path[len - 1] == VX_PATH_SEP)
    {
        path[--len] = '\0';
    }

    for (i32 i = (i32) len - 1; i >= 0; i--)
    {
        if (path[i] == VX_PATH_SEP)
        {
            if (i == 0)
            {
                path[1] = '\0';
            }
            else
            {
                path[i] = '\0';
            }

            return true;
        }
    }

    return false;
}

vx_sv vx_sv_from_cstr(const char *cstr)
{
    vx_sv sv;
    sv.data = cstr;
    sv.len  = strlen(cstr);
    return sv;
}

/**
 * Compares a vx_sv against a null-terminated C-string.
 * Returns 0 if they are identical in content and length.
 */
i32 vx_sv_strcmp(vx_sv sv, const char *cstr)
{
    if (cstr == nullptr)
    {
        return 1;
    }

    size_t cstr_len = strlen(cstr);

    // lens dont match cant be equal
    if (sv.len != cstr_len)
    {
        return (sv.len < cstr_len) ? -1 : 1;
    }

    return memcmp(sv.data, cstr, sv.len);
}
