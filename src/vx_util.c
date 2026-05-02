#include "vx_util.h"
#include "vx_limits.h"
#include "vx_platform.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#if defined(__linux__)
#include <time.h>
#endif

vx_status vx_mkdir_p(const char *path)
{
    char   tmp[VX_PATH_MAX];
    size_t len = strlen(path);

    if (len == 0 || len >= sizeof(tmp))
    {
        return VX_ERROR;
    }

    memcpy(tmp, path, len);
    tmp[len] = '\0';

    char *p = tmp;

    if (len > 2 && tmp[1] == ':')
    {
        p = tmp + 3;  // start after 'C:\'
    }
    else if (tmp[0] == '/' || tmp[0] == '\\')
    {
        p = tmp + 1;
    }

    for (; *p; ++p)
    {
        if (*p == '/' || *p == '\\')
        {
            char backup = *p;

            *p = '\0';

            if (strlen(tmp) > 0)
            {
                if (vx_mkdir(tmp) != 0 && errno != EEXIST)
                {
                    vx_stat_struct st;
                    if (vx_stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode))
                    {
                        vx_errlog("%s(): mkdir failed at: '%s'", __func__, tmp);
                        return VX_ERROR;
                    }
                }
            }

            *p = backup;
        }
    }

    if (vx_mkdir(tmp) != 0 && errno != EEXIST)
    {
        vx_stat_struct st;
        if (vx_stat(tmp, &st) != 0 || !S_ISDIR(st.st_mode))
        {
            vx_errlog("%s(): mkdir failed at: '%s'", __func__, tmp);
            return VX_ERROR;
        }
    }

    return VX_OK;
}

char *vx_trim_s(char *s)
{
    while (isspace((u8) *s))
    {
        s++;
    }

    if (*s == 0)
    {
        return s;
    }

    char *end = s + strlen(s) - 1;

    while (end > s && isspace((u8) *end))
    {
        *end = 0;
        end--;
    }

    return s;
}

void vx_yield(void)
{
#if defined(_WIN32) || defined(_WIN64)
    Sleep(0);
#else
    struct timespec ts = {.tv_sec = 0, .tv_nsec = 100 * 100};
    nanosleep(&ts, NULL);
#endif
}
