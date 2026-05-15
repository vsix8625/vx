#include "vx_fs.h"

#if defined(VX_OS_LINUX) && defined(VX_USE_THREADS)

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <dirent.h>
    #include <fcntl.h>
    #include <sys/sendfile.h>
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

bool vx_fs_cp(const char *src, const char *dest)
{
    if (src == nullptr || dest == nullptr)
    {
        return false;
    }

    i32 src_fd = open(src, O_RDONLY);

    if (src_fd < 0)
    {
        return false;
    }

    vx_stat_struct st;

    if (fstat(src_fd, &st) < 0)
    {
        close(src_fd);
        return false;
    }

    i32 dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode);

    if (dest_fd < 0)
    {
        close(src_fd);
        return false;
    }

    off_t   offset = 0;
    ssize_t sent   = sendfile(dest_fd, src_fd, &offset, st.st_size);

    close(src_fd);
    close(dest_fd);

    return sent == st.st_size;
}

bool vx_fs_ln(const char *src, const char *dest)
{
    if (src == nullptr || dest == nullptr)
    {
        return false;
    }

    unlink(dest);
    return link(src, dest) == 0;
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

vx_dir_handle vx_fs_dir_open(const char *path)
{
    return (vx_dir_handle) opendir(path);
}

bool vx_fs_dir_read(vx_dir_handle handle, vx_dir_entry *out_entry)
{
    DIR *d = (DIR *) handle;

    struct dirent *entry = readdir(d);

    if (entry == nullptr)
    {
        return false;
    }

    out_entry->name     = entry->d_name;
    out_entry->name_len = strlen(entry->d_name);

    out_entry->is_dir = (entry->d_type == DT_DIR);

    return true;
}

void vx_fs_dir_close(vx_dir_handle handle)
{
    if (handle)
    {
        closedir((DIR *) handle);
    }
}

#endif
