#include "vx_fs.h"

#if defined(VX_OS_LINUX) && defined(VX_USE_THREADS)

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <fcntl.h>
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

vx_sv vx_fs_read(const char *path, vx_alloc_fn alloc, void *user)
{
    vx_sv result = {0};

    bool use_default_alloc = alloc == nullptr;

    if (path == nullptr)
    {
        return result;
    }

    i32 fd = open(path, O_RDONLY);

    if (fd == -1)
    {
        VX_ASSERT_LOG("Failed to open %s", path);
        return result;
    }

    vx_stat_struct st;

    if (fstat(fd, &st) == -1)
    {
        VX_ASSERT_LOG("fstat failed for %s", path);
        return result;
    }

    if (st.st_size == 0)
    {
        close(fd);
        return result;
    }

    size_t len = (size_t) st.st_size;
    char  *buf = use_default_alloc ? vx_malloc(len + 1) : alloc(user, len + 1);

    if (buf == nullptr)
    {
        VX_ASSERT_LOG("Failed to allocate for %s", path);
        close(fd);
        return result;
    }

    ssize_t total = 0;
    while (total < (ssize_t) len)
    {
        ssize_t n = read(fd, buf + total, len - total);

        if (n <= 0)
        {
            VX_ASSERT_LOG("Failed to read %s", path);
            close(fd);

            if (use_default_alloc)
            {
                vx_free(buf);
            }
            return result;
        }
        total += n;
    }

    buf[len] = '\0';

    close(fd);

    result.data = buf;
    result.len  = (u32) len;

    return result;
}

//----------------------------------------------------------------------------------------------------

#endif
