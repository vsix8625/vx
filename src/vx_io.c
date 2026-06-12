#include "vx.h"

#include <linux/limits.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

//----------------------------------------------------------------------------------------------------

static char g_vx_color_string_pool[5][128];

static const char *g_ansi_color_codes[] = {[VX_COLOR_NONE]    = "",
                                           [VX_COLOR_GREEN]   = "38;5;40",
                                           [VX_COLOR_ORANGE]  = "38;5;202",
                                           [VX_COLOR_RED]     = "38;5;160",
                                           [VX_COLOR_CORAL]   = "38;5;167",
                                           [VX_COLOR_BLUE]    = "38;5;75",
                                           [VX_COLOR_MAGENTA] = "38;5;13"};

//----------------------------------------------------------------------------------------------------

static vx_log_mode g_vx_log_level = VX_LOG_ALL;

static vx_sv g_vx_plain_prefixes[5] = {[VX_LOG_LEVEL_PRINTF] = VX_SV(""),
                                       [VX_LOG_LEVEL_INFO]   = VX_SV("[log]: "),
                                       [VX_LOG_LEVEL_WARN]   = VX_SV("[warning]: "),
                                       [VX_LOG_LEVEL_ERROR]  = VX_SV("[error]: "),
                                       [VX_LOG_LEVEL_DEBUG]  = VX_SV("[debug]: ")};

static vx_sv g_vx_color_prefixes[5] = {
    [VX_LOG_LEVEL_PRINTF] = VX_SV(""),
    [VX_LOG_LEVEL_INFO]   = VX_SV("\033[2K\r\033[38;5;40m[log]: \033[0m"),
    [VX_LOG_LEVEL_WARN]   = VX_SV("\033[2K\r\033[38;5;202m[warning]: \033[0m"),
    [VX_LOG_LEVEL_ERROR]  = VX_SV("\033[2K\r\033[38;5;160m[error]: \033[0m"),
    [VX_LOG_LEVEL_DEBUG]  = VX_SV("\033[2K\r\033[38;5;167m[debug]: \033[0m")};

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
        if (g_stderr_tty == -1)
        {
            g_stderr_tty = vx_isatty(fd);
            return g_stderr_tty;
        }
    }

    return vx_isatty(fd);
}

// NOTE: deadlocked on MT
// NOTE: Testing lockfree logger for small buffs and using a 4096 buffer
static void vx_log_core(vx_log_type type, const char *fmt, va_list args)
{
    if (!vx_initialized() || fmt == nullptr)
    {
        return;
    }

    // Log level
    i32 fd = STDOUT_FILENO;
    if (type == VX_LOG_LEVEL_WARN || type == VX_LOG_LEVEL_ERROR)
    {
        fd = STDERR_FILENO;
    }

    if (g_vx_log_level == VX_LOG_QUIET)
    {
        if (type == VX_LOG_LEVEL_INFO || type == VX_LOG_LEVEL_DEBUG)
        {
            return;
        }
    }

    bool use_color = (vx_is_tty(fd) != 0);

    vx_sv prefix = use_color ? g_vx_color_prefixes[type] : g_vx_plain_prefixes[type];

    // end of setup

    char buf[VX_BUF_SIZE_4096];
    memcpy(buf, prefix.data, prefix.len);

    va_list aq;
    va_copy(aq, args);
    i32 msg_len = vsnprintf(buf + prefix.len, sizeof(buf) - prefix.len - 1, fmt, aq);
    va_end(aq);

    size_t total_len = prefix.len + (size_t) msg_len;

    if (total_len < sizeof(buf) - 1)
    {
        if (type != VX_LOG_LEVEL_PRINTF)
        {
            buf[total_len++] = '\n';
        }
        vx_write(fd, buf, total_len);
    }
    else
    {
        while (atomic_flag_test_and_set(&g_io_atomic_lock))
        {
            vx_yield();
        }

        char *big = vx_malloc(total_len + 2);

        if (big)
        {
            memcpy(big, prefix.data, prefix.len);

            va_list aq_heap;
            va_copy(aq_heap, args);
            vsnprintf(big + prefix.len, (size_t) msg_len + 1, fmt, aq_heap);
            va_end(aq_heap);

            if (type != VX_LOG_LEVEL_PRINTF)
            {
                big[total_len] = '\n';
                total_len++;
            }

            vx_write(fd, big, total_len);
            vx_free(big);
        }
        atomic_flag_clear(&g_io_atomic_lock);
    }
}

void vx_printf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LOG_LEVEL_PRINTF, fmt, args);
    va_end(args);
}

void vx_warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LOG_LEVEL_WARN, fmt, args);
    va_end(args);
}

void vx_errlog(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LOG_LEVEL_ERROR, fmt, args);
    va_end(args);
}

void vx_log(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vx_log_core(VX_LOG_LEVEL_INFO, fmt, args);
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
    vx_log_core(VX_LOG_LEVEL_DEBUG, fmt, args);
    va_end(args);
}

//----------------------------------------------------------------------------------------------------

vx_status vx_fwrite(const char *path, const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return VX_LIB_NOT_INITIALIZED;
    }

    if (path == nullptr || fmt == nullptr)
    {
        return VX_ERROR;
    }

    while (atomic_flag_test_and_set(&g_fwrite_atomic_lock))
    {
        vx_yield();
    }

    FILE *fp = fopen(path, "w");

    if (fp == nullptr)
    {
        vx_errlog("%s: Failed to open: %s", __func__, path);
        atomic_flag_clear(&g_fwrite_atomic_lock);
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

    if (fp == nullptr)
    {
        vx_errlog("%s: Failed to open: %s", __func__, path);
        atomic_flag_clear(&g_fwrite_atomic_lock);
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
    if (!vx_initialized() || buf == nullptr || buf->offset >= buf->size)
    {
        return;
    }

    va_list args;
    va_start(args, fmt);

    size_t remaining = buf->size - buf->offset;

    i32 written = vsnprintf(buf->data + buf->offset, remaining, fmt, args);

    va_end(args);

    if (written > 0)
    {
        size_t actual_added = ((size_t) written < remaining) ? (size_t) written : (remaining - 1);

        buf->offset += actual_added;
    }
}

void vx_log_set_level(vx_log_mode mode)
{
    g_vx_log_level = mode;
}

//----------------------------------------------------------------------------------------------------

void vx_io_set_prefix(vx_log_type type, const char *prefix, vx_color color)
{
    if (type < VX_LOG_LEVEL_INFO || type > VX_LOG_LEVEL_DEBUG || prefix == nullptr)
    {
        return;
    }

    g_vx_plain_prefixes[type] = vx_sv_from_cstr(prefix);

    if (color != VX_COLOR_NONE && color < (sizeof(g_ansi_color_codes) / sizeof(char *)))
    {
        char temp_buf[128];

        i32 len = snprintf(temp_buf,
                           sizeof(temp_buf),
                           "\033[2K\r\033[%sm%s\033[0m",
                           g_ansi_color_codes[color],
                           prefix);

        if (len > 0 && (size_t) len < sizeof(temp_buf))
        {
            memcpy(g_vx_color_string_pool[type], temp_buf, (size_t) len + 1);

            g_vx_color_prefixes[type].data = g_vx_color_string_pool[type];
            g_vx_color_prefixes[type].len  = (size_t) len;
        }
        else
        {
            g_vx_color_prefixes[type] = g_vx_plain_prefixes[type];
        }
    }
    else
    {
        g_vx_color_prefixes[type] = g_vx_plain_prefixes[type];
    }
}
