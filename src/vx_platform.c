#include "vx_platform.h"
#include "vx_limits.h"

#include <stdio.h>
#include <stdlib.h>

const char *vx_platform_get_cache_dir(void)
{
    static char cache_path[VX_PATH_MAX];

    if (cache_path[0] != '\0')
    {
        return cache_path;
    }

#if defined(VX_OS_WINDOWS)
    const char *ldata = getenv("LOCALAPPDATA");
    if (ldata != nullptr && ldata[0] != '\0')
    {
        return ldata;
    }
    return nullptr;
#else
    const char *xdg = getenv("XDG_CACHE_HOME");
    if (xdg != nullptr && xdg[0] != '\0')
    {
        return xdg;
    }

    const char *home = vx_platform_get_home_dir();
    if (home != nullptr)
    {
    #if defined(VX_OS_MACOS)
        snprintf(cache_path, sizeof(cache_path), "%s/Library/Caches", home);
    #else
        snprintf(cache_path, sizeof(cache_path), "%s/.cache", home);
    #endif
        return cache_path;
    }
    return nullptr;
#endif
}

const char *vx_platform_get_config_dir(void)
{
    static char config_path[VX_PATH_MAX];

#if defined(VX_OS_WINDOWS)
    const char *appdata = getenv("APPDATA");
    if (appdata != nullptr && appdata[0] != '\0')
    {
        return appdata;
    }
    return nullptr;
#else
    const char *xdg = getenv("XDG_CONFIG_HOME");
    if (xdg != nullptr && xdg[0] != '\0')
    {
        return xdg;
    }

    const char *home = getenv("HOME");
    if (home != nullptr && home[0] != '\0')
    {
    #if defined(VX_OS_MACOS)
        snprintf(config_path, sizeof(config_path), "%s/Library/Preferences", home);
    #else
        snprintf(config_path, sizeof(config_path), "%s/.config", home);
    #endif
        return config_path;
    }
    return nullptr;
#endif
}

const char *vx_platform_get_home_dir(void)
{
#if defined(VX_OS_WINDOWS)
    const char *uprofile = getenv("USERPROFILE");
    if (uprofile != nullptr && uprofile[0] != '\0')
    {
        return uprofile;
    }
    return nullptr;
#else
    const char *home = getenv("HOME");
    if (home != nullptr && home[0] != '\0')
    {
        return home;
    }

    struct passwd *pw = getpwuid(getuid());
    return pw ? pw->pw_dir : nullptr;
#endif
}

vx_status vx_platform_setenv(const char *name, const char *value)
{
#if defined(VX_OS_WINDOWS)
    return _putenv_s(name, value) == 0 ? VX_OK : VX_ERROR;
#else
    return setenv(name, value, 1) == 0 ? VX_OK : VX_ERROR;
#endif
}

vx_status vx_platform_get_self_exe(char *out_buf, u32 buf_size)
{
    if (out_buf == nullptr || buf_size == 0)
    {
        return VX_ERROR;
    }

#if defined(VX_OS_WINDOWS)

    DWORD len = GetModuleFileNameA(NULL, out_buf, (DWORD) buf_size);

    if (len == 0 || len >= (DWORD) buf_size)
    {
        return VX_ERROR;
    }

#elif defined(VX_OS_MACOS)

    u32 size = (u32) buf_size;
    if (_NSGetExecutablePath(out_buf, &size) != 0)
    {
        return VX_ERROR;
    }

#else

    ssize_t len = readlink("/proc/self/exe", out_buf, (size_t) (buf_size - 1));
    if (len == -1)
    {
        return VX_ERROR;
    }
    out_buf[len] = '\0';

#endif

    return VX_OK;
}
