#include "vx_fs.h"
#include "vx_string.h"
#include "vx_util.h"

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
    if (path == nullptr || g_vx_fs_forbidden_count >= VX_FS_FORBIDDEN_PATH_MAX_COUNT)
    {
        return;
    }

    char resolved[VX_PATH_MAX];

    if (vx_fs_realpath(path, resolved) == nullptr)
    {
        return;
    }

    strncpy(g_vx_fs_forbidden_paths[g_vx_fs_forbidden_count], resolved, VX_PATH_MAX - 1);

    g_vx_fs_forbidden_paths[g_vx_fs_forbidden_count][VX_PATH_MAX - 1] = '\0';

    g_vx_fs_forbidden_count++;
}

bool vx_fs_is_path_protected(const char *target)
{
    if (target == nullptr)
    {
        return true;
    }

    for (u32 i = 0; i < g_vx_fs_forbidden_count; i++)
    {
        const char *restricted = g_vx_fs_forbidden_paths[i];

        size_t r_len = strlen(restricted);
        size_t t_len = strlen(target);

        if (vx_pathncmp(restricted, target, r_len) == 0)
        {
            if (t_len == r_len || target[r_len] == VX_PATH_SEP)
            {
                return true;
            }
        }
    }

#if defined(VX_OS_WINDOWS)
    const char *defaults[] = {"C:\\", "C:\\Windows", "C:\\Users", nullptr};
#else
    const char *defaults[] = {"/", "/etc", "/usr", "/bin", "/boot", nullptr};
#endif

    for (u32 i = 0; defaults[i]; ++i)
    {
        size_t r_len = strlen(defaults[i]);

        if (vx_pathncmp(defaults[i], target, r_len) == 0)
        {
            if (strlen(target) == r_len || target[r_len] == VX_PATH_SEP)
            {
                return true;
            }
        }
    }

    return false;
}

//----------------------------------------------------------------------------------------------------
