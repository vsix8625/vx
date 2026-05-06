#include "vx_fs.h"

#if defined(VX_OS_WINDOWS)

    #include "vx_platform.h"
    #include "vx_limits.h"

    #include <stdlib.h>

bool vx_fs_mv(const char *src, const char *dest)
{
    wchar_t w_src[VX_PATH_MAX], w_dest[VX_PATH_MAX];

    MultiByteToWideChar(CP_UTF8, 0, src, -1, w_src, VX_PATH_MAX);
    MultiByteToWideChar(CP_UTF8, 0, dest, -1, w_dest, VX_PATH_MAX);

    return MoveFileExW(w_src, w_dest, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0;
}

char *vx_fs_realpath(const char *path, char *resolved)
{
    if (path == nullptr)
    {
        return nullptr;
    }

    wchar_t w_path[VX_PATH_MAX];
    wchar_t w_res[VX_PATH_MAX];

    i32 w_len = MultiByteToWideChar(CP_UTF8, 0, path, -1, w_path, VX_PATH_MAX);
    if (w_len <= 0)
    {
        return nullptr;
    }

    if (_wfullpath(w_res, w_path, VX_PATH_MAX) == nullptr)
    {
        return nullptr;
    }

    char *out = resolved;

    if (out == nullptr)
    {
        out = (char *) vx_malloc(VX_PATH_MAX);

        if (out == nullptr)
        {
            return nullptr;
        }
    }

    i32 mb_len = WideCharToMultiByte(CP_UTF8, 0, w_res, -1, out, VX_PATH_MAX, nullptr, nullptr);
    if (mb_len <= 0)
    {
        if (resolved == nullptr)
        {
            vx_free(out);
        }
        return nullptr;
    }

    return out;
}

static bool vx_win32_rm_internal(const wchar_t *w_path)
{
    wchar_t          search_path[VX_PATH_MAX];
    WIN32_FIND_DATAW fd;

    swprintf(search_path, VX_PATH_MAX, L"%s\\*", w_path);

    HANDLE h_find = FindFirstFileW(search_path, &fd);
    if (h_find == INVALID_HANDLE_VALUE)
    {
        return GetLastError() == ERROR_PATH_NOT_FOUND || GetLastError() == ERROR_FILE_NOT_FOUND;
    }

    bool success = true;

    while (true)
    {
        const wchar_t *name = fd.cFileName;

        if (wcscmp(name, L".") != 0 && wcscmp(name, L"..") != 0)
        {
            wchar_t child_path[VX_PATH_MAX];
            swprintf(child_path, VX_PATH_MAX, L"%s\\%s", w_path, name);

            if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                if (!vx_win32_rm_internal(child_path))
                {
                    success = false;
                }
            }
            else
            {
                // force normal attributes if it's read-only so we can nuke it
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
                {
                    SetFileAttributesW(child_path, FILE_ATTRIBUTE_NORMAL);
                }

                if (!DeleteFileW(child_path))
                {
                    success = false;
                }
            }
        }

        if (!FindNextFileW(h_find, &fd))
        {
            break;
        }
    }

    FindClose(h_find);

    if (success)
    {
        // remove the empty directory
        if (!RemoveDirectoryW(w_path))
        {
            success = false;
        }
    }

    return success;
}

bool vx_fs_rmrf(const char *path)
{
    if (path == nullptr)
    {
        return false;
    }

    char resolved[VX_PATH_MAX];
    if (vx_fs_realpath(path, resolved) == nullptr)
    {
        return true;
    }

    if (vx_fs_is_path_protected(resolved))
    {
        vx_errlog("Cannot remove (protected): %s", resolved);
        return false;
    }

    wchar_t w_path[VX_PATH_MAX];
    i32     w_len = MultiByteToWideChar(CP_UTF8, 0, resolved, -1, w_path, VX_PATH_MAX);
    if (w_len <= 0)
    {
        return false;
    }

    return vx_win32_rm_internal(w_path);
}

#endif
