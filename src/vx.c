#include "vx.h"
#include <stdatomic.h>

static atomic_int g_vx_initialized = 0;

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
