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

    for (char *p = tmp + 1; *p; ++p)
    {
        if (*p == '/' || *p == '\\')
        {
            char backup = *p;

            *p = '\0';

            if (p != tmp && (strlen(tmp) > 0 && tmp[len - 1] != ':'))
            {
                if (vx_mkdir(tmp) != 0 && errno != EEXIST)
                {
                    vx_errlog("%s(): mkdir failed", __func__);
                    return VX_ERROR;
                }
            }

            *p = backup;
        }
    }

    if (vx_mkdir(tmp) != 0 && errno != EEXIST)
    {
        vx_errlog("%s(): mkdir failed", __func__);
        return VX_ERROR;
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
