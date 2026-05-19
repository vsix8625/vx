#ifndef VX_FS_H_
#define VX_FS_H_

#include "vx_platform.h"
#include "vx_string.h"
#include "vx_io.h"

#include <ctype.h>
#include <stdio.h>

/*
 * Move/Rename `src` to `dest`.
 * Returns `true` on success, or `false` on failure.
 */
VX_API bool vx_fs_mv(const char *src, const char *dest);

VX_API bool vx_fs_cp(const char *src, const char *dest);

/*
 * Creates a hard link from src to dest.
 * If replace is true, removes dest before linking (safe overwrite).
 * If replace is false, fails if dest already exists.
 * Uses link() syscall — both paths must be on the same filesystem.
 *
 * @param src     the existing file to link from.
 * @param dest    the new directory entry to create.
 * @param replace if true, unlinks dest before creating the link.
 * @return true on success, false otherwise.
 */
VX_API bool vx_fs_ln(const char *src, const char *dest, bool replace);

/*
 * Uses `realpath` from `<stdlib.h>` in `Linux` and `_wfullpath` in `Windows`.
 *
 * NOTE: If resolved is `nullptr` it will return a heap allocated buffer, caller is responsible
 * to free().
 */
VX_API char *vx_fs_realpath(const char *path, char *resolved);

VX_API bool vx_fs_rmrf(const char *path);

/*
 * If alloc is nullptr, uses `vx_malloc`. Caller must vx_free result.data when done.
 * If alloc is provided, memory lifetime is managed by the allocator.
 * */
VX_API vx_sv vx_fs_read(const char *path, vx_alloc_fn alloc, void *user);

//----------------------------------------------------------------------------------------------------

static inline bool vx_fs_is_dot_dir(const char *name, size_t len)
{
    return vx_strncmplit(name, len, ".", 1) || vx_strncmplit(name, len, "..", 2);
}

VX_API vx_dir_handle vx_fs_dir_open(const char *path);
VX_API bool          vx_fs_dir_read(vx_dir_handle handle, vx_dir_entry *out_entry);
VX_API void          vx_fs_dir_close(vx_dir_handle handle);

// TODO: win32 version
VX_API vx_status vx_fs_which(const char *name, char *out_path, size_t out_size);

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
VX_API bool vx_fs_is_path_protected(const char *target);  // TODO: include win32 in desc

VX_API void vx_fs_log_forbidden_paths(void);

VX_API vx_status vx_fs_get_file_metrics(const char *path, u64 *out_size, u64 *out_mtime);

//----------------------------------------------------------------------------------------------------

static inline void vx_clear_term(void)
{
    vx_printf("\033[1;1H\033[2J");
}

static inline bool vx_isdir(const char *path)
{
    if (path == NULL)
    {
        return false;
    }

    vx_stat_struct st;
    return (vx_stat(path, &st) == 0 && S_ISDIR(st.st_mode));
}

static inline bool vx_isfile(const char *path)
{
    FILE *f = fopen(path, "r");

    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

/*
 * Thread local buf
 */
static inline const char *vx_getcwd_fn(void)
{
    static thread_local char buf[VX_PATH_MAX];

    if (vx_getcwd(buf, sizeof(buf)) == NULL)
    {
        return NULL;
    }

    return buf;
}

static inline bool vx_fs_is_abspath(const char *path)
{
    if (path == nullptr || path[0] == '\0')
    {
        return false;
    }

#if defined(VX_OS_WINDOWS)

    if (isalpha((u8) path[0]) && path[1] == ':')
    {
        return path[2] == '\\' || path[2] == '/';
    }

    if ((path[0] == '\\' && path[1] == '\\') || (path[0] == '/' && path[1] == '/'))
    {
        return true;
    }

#else
    return path[0] == '/';
#endif
}

#endif  // VX_FS_H_
