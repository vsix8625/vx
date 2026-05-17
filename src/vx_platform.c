#include "vx_platform.h"
#include "vx_limits.h"

#include <stdio.h>
#include <stdlib.h>

const char *vx_platform_get_cache_dir(void)
{
    static char cache_path[VX_PATH_MAX];

#if defined(VX_OS_WINDOWS)
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

const char *vx_platform_get_config_dir(void)
{
    static char config_path[VX_PATH_MAX];

#if defined(VX_OS_WINDOWS)
    return getenv("APPDATA");
#else
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg != nullptr)
    {
        return xdg;
    }

    const char *home = getenv("HOME");
    if (home != nullptr)
    {
        snprintf(config_path, sizeof(config_path), "%s/.config", home);
        return config_path;
    }
    return nullptr;
#endif
}
