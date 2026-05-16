#include "vx.h"
#include <stdatomic.h>

static atomic_int g_vx_initialized = 0;

static void vx_io_init(void);

bool vx_initialized(void)
{
    return atomic_load(&g_vx_initialized);
}

vx_status vx_init(void)
{
    if (atomic_load(&g_vx_initialized))
    {
        // already initialized
        return VX_ERROR;
    }

    vx_io_init();
    vx_init_time();

    atomic_store(&g_vx_initialized, 1);
    return VX_OK;
}

void vx_shutdown(void)
{
    atomic_store(&g_vx_initialized, 0);
}

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

static void vx_io_init(void)
{
#if defined(VX_OS_WINDOWS)
    vx_win32_io_init();
#endif
}
