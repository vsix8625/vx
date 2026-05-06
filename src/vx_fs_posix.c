#include "vx_fs.h"

#if defined(VX_OS_LINUX)

    #include <stdio.h>

bool vx_fs_mv(const char *src, const char *dest)
{
    if (src == nullptr || dest == nullptr)
    {
        return false;
    }

    return rename(src, dest) == 0;
}

#endif
