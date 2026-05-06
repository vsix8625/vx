#include "vx_fs.h"

#if defined(VX_OS_WINDOWS)

bool vx_fs_mv(const char *src, const char *dest)
{
    wchar_t w_src[VX_PATH_MAX], w_dest[VX_PATH_MAX];

    MultiByteToWideChar(CP_UTF8, 0, src, -1, w_src, VX_PATH_MAX);
    MultiByteToWideChar(CP_UTF8, 0, dest, -1, w_dest, VX_PATH_MAX);

    return MoveFileExW(w_src, w_dest, MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED) != 0;
}

#endif
