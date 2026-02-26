#include "vx.h"
#include <stdatomic.h>

static atomic_int g_vx_initialized = 0;

bool vx_initialized(void)
{
    if (!atomic_load(&g_vx_initialized))
    {
        fprintf(stderr, "%s: VX library is not initialized\n", __func__);
        return false;
    }
    return true;
}

vx_status vx_init(void)
{
    if (atomic_load(&g_vx_initialized))
    {
        fprintf(stderr, "VX library is already initialized\n");
        return VX_ERROR;
    }

    atomic_store(&g_vx_initialized, 1);
    return VX_OK;
}

vx_status vx_shutdown(void)
{
    if (!atomic_load(&g_vx_initialized))
    {
        fprintf(stderr, "%s: VX library is not initialized\n", __func__);
        return VX_ERROR;
    }

    atomic_store(&g_vx_initialized, 0);
    return VX_OK;
}
