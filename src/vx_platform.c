#include "vx_platform.h"
#include "vx_limits.h"
#include <stdio.h>
#include <stdlib.h>

#if defined(VX_OS_WINDOWS)
static void vx_win32_io_init(void)
{
    DWORD handles[] = {STD_OUTPUT_HANDLE, STD_ERROR_HANDLE};
    for (int i = 0; i < 2; i++)
    {
        HANDLE h = GetStdHandle(handles[i]);
        if (h != INVALID_HANDLE_VALUE)
        {
            DWORD dwMode = 0;
            if (GetConsoleMode(h, &dwMode))
            {
                dwMode |= 0x0004;  // ENABLE_VIRTUAL_TERMINAL_PROCESSING
                SetConsoleMode(h, dwMode);
            }
        }
    }
}
#endif

void vx_io_init(void)
{
#if defined(VX_OS_WINDOWS)
    vx_win32_io_init();
#endif
}

const char *vx_platform_get_cache_dir(void)
{
    static char cache_path[VX_PATH_MAX];

#if defined(VX_OS_WINODWS)
    return getenv("LOCALAPPDATA");
#else
    const char *xdg = getenv("XDG_CACHE_HOME");

    if (xdg != nullptr)
    {
        return xdg;
    }

    const char *home = getenv("HOME");
    if (home != nullptr)
    {
        snprintf(cache_path, sizeof(cache_path), "%s/.cache", home);
        return cache_path;
    }
    return nullptr;
#endif
}
