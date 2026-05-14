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

bool vx_fs_cp(const char *src, const char *dest)
{
    wchar_t w_src[VX_PATH_MAX], w_dest[VX_PATH_MAX];

    MultiByteToWideChar(CP_UTF8, 0, src, -1, w_src, VX_PATH_MAX);
    MultiByteToWideChar(CP_UTF8, 0, dest, -1, w_dest, VX_PATH_MAX);

    return CopyFileW(w_src, w_dest, FALSE) != 0;
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

//----------------------------------------------------------------------------------------------------

vx_sv vx_fs_read(const char *path, vx_alloc_fn alloc, void *user)
{
    vx_sv result            = {0};
    bool  use_default_alloc = alloc == nullptr;

    if (path == nullptr)
    {
        return result;
    }

    HANDLE h = CreateFileA(path,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           nullptr,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           nullptr);

    if (h == INVALID_HANDLE_VALUE)
    {
        VX_ASSERT_LOG("Failed to open %s", path);
        return result;
    }

    LARGE_INTEGER size;

    if (!GetFileSizeEx(h, &size))
    {
        VX_ASSERT_LOG("GetFileSizeEx failed for %s", path);
        CloseHandle(h);
        return result;
    }

    if (size.QuadPart == 0)
    {
        CloseHandle(h);
        return result;
    }

    size_t len = (size_t) size.QuadPart;
    char  *buf = use_default_alloc ? vx_malloc(len + 1) : alloc(user, len + 1);

    if (buf == nullptr)
    {
        VX_ASSERT_LOG("Failed to allocate for %s", path);
        CloseHandle(h);
        return result;
    }

    DWORD read_bytes = 0;

    if (!ReadFile(h, buf, (DWORD) len, &read_bytes, nullptr) || read_bytes != (DWORD) len)
    {
        VX_ASSERT_LOG("ReadFile failed for %s", path);
        CloseHandle(h);

        if (use_default_alloc)
        {
            vx_free(buf);
        }

        return result;
    }

    buf[len] = '\0';
    CloseHandle(h);

    result.data = buf;
    result.len  = (u32) len;

    return result;
}

//----------------------------------------------------------------------------------------------------

typedef struct
{
    HANDLE          h;
    WIN32_FIND_DATA data;
    bool            first;
} vx_win32_dir;

vx_dir_handle vx_fs_dir_open(const char *path)
{
    char buf[MAX_PATH];
    snprintf(buf, MAX_PATH, "%s\\*", path);

    vx_win32_dir *w = vx_malloc(sizeof(vx_win32_dir));
    w->h            = FindFirstFileA(buf, &w->data);

    if (w->h == INVALID_HANDLE_VALUE)
    {
        vx_free(w);
        return NULL;
    }

    w->first = true;
    return (vx_dir_handle) w;
}

bool vx_fs_dir_read(vx_dir_handle handle, vx_dir_entry *out_entry)
{
    vx_win32_dir *w = (vx_win32_dir *) handle;

    if (!w->first)
    {
        if (!FindNextFileA(w->h, &w->data))
        {
            return false;
        }
    }
    w->first = false;

    out_entry->name     = w->data.cFileName;
    out_entry->name_len = strlen(out_entry->name);
    out_entry->is_dir   = (w->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);

    return true;
}

void vx_fs_dir_close(vx_dir_handle handle)
{
    if (!handle)
    {
        return;
    }

    vx_win32_dir *w = (vx_win32_dir *) handle;
    FindClose(w->h);
    vx_free(w);
}

//----------------------------------------------------------------------------------------------------

#endif
