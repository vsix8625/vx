#include "vx_io.h"
#include "vx.h"
#include "vx_util.h"

#include <stdarg.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    VX_LEVEL_PRINTF,
    VX_LEVEL_INFO,
    VX_LEVEL_WARN,
    VX_LEVEL_ERROR,
    VX_LEVEL_DEBUG,
} vx_log_type;

static const vx_sv vx_prefix_none  = VX_SV("");
static const vx_sv vx_prefix_log   = VX_SV("\033[38;5;40m[log]: \033[0m");
static const vx_sv vx_prefix_warn  = VX_SV("\033[38;5;202m[warning]: \033[0m");
static const vx_sv vx_prefix_error = VX_SV("\033[38;5;160m[error]: \033[0m");
static const vx_sv vx_prefix_debug = VX_SV("\033[38;5;167m[debug]: \033[0m");

static const vx_sv vx_plain_log   = VX_SV("[log]: ");
static const vx_sv vx_plain_warn  = VX_SV("[warning]: ");
static const vx_sv vx_plain_error = VX_SV("[error]: ");
static const vx_sv vx_plain_debug = VX_SV("[debug]: ");

static atomic_flag g_io_atomic_lock     = ATOMIC_FLAG_INIT;
static atomic_flag g_fwrite_atomic_lock = ATOMIC_FLAG_INIT;

static i32 g_stdout_tty = -1;
static i32 g_stderr_tty = -1;

static void vx_log_core(vx_log_type type, const char *fmt, va_list args)
    __attribute__((format(__printf__, 2, 0)));

static inline bool vx_is_tty(i32 fd)
{
    if (fd == STDOUT_FILENO)
    {
        if (g_stdout_tty == -1)
        {
            g_stdout_tty = vx_isatty(fd);
            return g_stdout_tty;
        }
    }

    if (fd == STDERR_FILENO)
    {
        if (g_stdout_tty == -1)
        {
            g_stderr_tty = vx_isatty(fd);
            return g_stderr_tty;
        }
    }

    return vx_isatty(fd);
}

static void vx_log_core(vx_log_type type, const char *fmt, va_list args)
{
    if (!vx_initialized() || fmt == NULL)
    {
        return;
    }

    i32 fd = STDOUT_FILENO;

    if (type != VX_LEVEL_PRINTF)
    {
        fd = STDERR_FILENO;
    }

    bool use_color = (vx_is_tty(fd) != 0);

    vx_sv prefix = vx_prefix_none;
    if (use_color)
    {
        switch (type)
        {
            case VX_LEVEL_INFO:
            {
                prefix = vx_prefix_log;
                break;
            }

            case VX_LEVEL_WARN:
            {
                prefix = vx_prefix_warn;
                break;
            }

            case VX_LEVEL_ERROR:
            {
                prefix = vx_prefix_error;
                break;
            }

            case VX_LEVEL_DEBUG:
            {
                prefix = vx_prefix_debug;
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        switch (type)
        {
            case VX_LEVEL_INFO:
            {
                prefix = vx_plain_log;
                break;
            }

            case VX_LEVEL_WARN:
            {
                prefix = vx_plain_warn;
                break;
            }

            case VX_LEVEL_ERROR:
            {
                prefix = vx_plain_error;
                break;
            }

            case VX_LEVEL_DEBUG:
            {
                prefix = vx_plain_debug;
                break;
            }

            default:
            {
                break;
            }
        }
    }

    // end of setup

    while (atomic_flag_test_and_set(&g_io_atomic_lock))
    {
        vx_yield();
    }

    char buf[VX_PATH_MAX];
    memcpy(buf, prefix.data, prefix.len);

    va_list aq;
    va_copy(aq, args);
    i32 msg_len = vsnprintf(buf + prefix.len, sizeof(buf) - prefix.len - 1, fmt, aq);
    va_end(aq);

    if (msg_len < 0)
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    size_t total_len = prefix.len + (size_t) msg_len;

    if (total_len < sizeof(buf) - 1)
    {
        if (type != VX_LEVEL_PRINTF)
            buf[total_len++] = '\n';
        vx_write(fd, buf, total_len);
    }
    else
    {
        char *big = vx_malloc(total_len + 2);
        if (big)
        {
            memcpy(big, prefix.data, prefix.len);
            vsnprintf(big + prefix.len, (size_t) msg_len + 1, fmt, args);

            if (type != VX_LEVEL_PRINTF)
            {
                big[total_len] = '\n';
                total_len++;
            }

            vx_write(fd, big, total_len);
            vx_free(big);
        }
    }

    atomic_flag_clear(&g_io_atomic_lock);
}

void vx_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LEVEL_PRINTF, fmt, args);
    va_end(args);
}

void vx_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LEVEL_WARN, fmt, args);
    va_end(args);
}

void vx_errlog(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LEVEL_ERROR, fmt, args);
    va_end(args);
}

void vx_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LEVEL_INFO, fmt, args);
    va_end(args);
}

static bool g_vx_set_debug = false;

void vx_set_debug(bool enabled)
{
    g_vx_set_debug = enabled;
}

void vx_dbglog(const char *fmt, ...)
{
    if (g_vx_set_debug == false)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

//----------------------------------------------------------------------------------------------------

vx_status vx_fwrite(const char *path, const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return VX_LIB_NOT_INITIALIZED;
    }

    if (path == NULL || fmt == NULL)
    {
        return VX_ERROR;
    }

    while (atomic_flag_test_and_set(&g_fwrite_atomic_lock))
    {
        vx_yield();
    }

    FILE *fp = fopen(path, "w");

    if (!fp)
    {
        vx_errlog("Failed to open: %s", path);
        return VX_ERROR;
    }

    va_list args;
    va_start(args, fmt);
    i32 result = vfprintf(fp, fmt, args);
    va_end(args);

    fclose(fp);

    atomic_flag_clear(&g_fwrite_atomic_lock);
    return result >= 0 ? VX_OK : VX_ERROR;
}

vx_status vx_fappend(const char *path, const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return VX_LIB_NOT_INITIALIZED;
    }

    while (atomic_flag_test_and_set(&g_fwrite_atomic_lock))
    {
        vx_yield();
    }

    FILE *fp = fopen(path, "a");
    if (!fp)
    {
        vx_errlog("Failed to open: %s", path);
        return VX_ERROR;
    }

    va_list args;
    va_start(args, fmt);
    i32 result = vfprintf(fp, fmt, args);
    va_end(args);

    fclose(fp);

    atomic_flag_clear(&g_fwrite_atomic_lock);
    return result >= 0 ? VX_OK : VX_ERROR;
}

void vx_sbuf_append(vx_sbuf *buf, const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (buf == nullptr || buf->offset >= buf->size)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    i32 written = vsnprintf(buf->data + buf->offset, buf->size - buf->offset, fmt, args);

    va_end(args);

    if (written > 0)
    {
        buf->offset += (size_t) written;
        if (buf->offset >= buf->size)
        {
            buf->offset = buf->size - 1;
        }
    }
}
