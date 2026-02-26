#pragma GCC system_header
#ifndef IO_H_
#define IO_H_

#include "limits.h"
#include "defs.h"

#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#define VA_CHECK(fmt_arg_n) __attribute__((format(__printf__, fmt_arg_n, fmt_arg_n + 1)))

void vx_printf(const char *fmt, ...) VA_CHECK(1);
void vx_warn(const char *fmt, ...) VA_CHECK(1);
void vx_errlog(const char *fmt, ...) VA_CHECK(1);
void vx_log(const char *fmt, ...) VA_CHECK(1);
void vx_dbglog(const char *fmt, ...) VA_CHECK(1);

//----------------------------------------------------------------------------------------------------

static inline void vx_clear_term(void)
{
    vx_printf("\e[1;1H\e[2J");
}

static inline bool vx_isdir(const char *dir)
{
    struct stat st;
    return (stat(dir, &st) == 0 && S_ISDIR(st.st_mode));
}

static inline bool vx_isfile(const char *path)
{
    FILE *f = fopen(path, "r");
    if (f)
    {
        fclose(f);
        return true;
    }
    return false;
}

static inline const char *vx_getcwd(void)
{
    static char buf[VX_PATH_MAX];
    return getcwd(buf, sizeof(buf));
}

//----------------------------------------------------------------------------------------------------

#endif  // IO_H_
