#include "vx_fs.h"

#if defined(VX_OS_LINUX)

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <ftw.h>

    #include "vx_string.h"
    #include "vx_util.h"

bool vx_fs_mv(const char *src, const char *dest)
{
    if (src == nullptr || dest == nullptr)
    {
        return false;
    }

    return rename(src, dest) == 0;
}

char *vx_fs_realpath(const char *path, char *resolved)
{
    if (path == nullptr)
    {
        return nullptr;
    }

    return realpath(path, resolved);
}

static i32
nftw_remove_cb(const char *path, const vx_stat_struct *sb, i32 typeflag, struct FTW *ftwbuf)
{
    VX_CAST(void, sb);
    VX_CAST(void, typeflag);
    VX_CAST(void, ftwbuf);

    return VX_CAST(i32, remove(path));
}

//----------------------------------------------------------------------------------------------------

bool vx_fs_rmrf(const char *path)
{
    if (path == nullptr)
    {
        return false;
    }

    char resolved[VX_PATH_MAX];
    if (vx_fs_realpath(path, resolved) == nullptr)
    {
        return false;
    }

    if (vx_fs_is_path_protected(resolved))
    {
        vx_errlog("Cannot remove (protected): %s", resolved);
        return false;
    }

    if (nftw(resolved, nftw_remove_cb, 64, FTW_DEPTH | FTW_PHYS) != 0)
    {
        return false;
    }

    return true;
}

//----------------------------------------------------------------------------------------------------

#endif
