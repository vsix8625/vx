#include "vx_io.h"
#include "vx.h"

#include <stdarg.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static atomic_flag g_io_atomic_lock     = ATOMIC_FLAG_INIT;
static atomic_flag g_fwrite_atomic_lock = ATOMIC_FLAG_INIT;

static struct timespec g_io_ts = {.tv_sec = 0, .tv_nsec = 100 * 100};

void vx_printf(const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (fmt == NULL)
    {
        return;
    }

    while (atomic_flag_test_and_set(&g_io_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
    }

    char buf[VX_PATH_MAX];

    va_list args;
    va_start(args, fmt);
    i32 len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len < 0)
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    if (len < (i32) sizeof(buf))
    {
        write(STDOUT_FILENO, buf, (size_t) len);
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    char *big = vx_malloc((size_t) len + 1);

    if (big == NULL)
    {
        char errbuf[256];
        i32  errlen = snprintf(errbuf,
                               sizeof(errbuf),
                               "%s():%d: failed to allocate memory for buffer",
                               __func__,
                               __LINE__);
        if (errlen > 0)
        {
            write(STDERR_FILENO,
                  errbuf,
                  (errlen < (int) sizeof(errbuf)) ? (size_t) errlen : sizeof(errbuf) - 1);
        }
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    va_start(args, fmt);
    vsnprintf(big, (size_t) len + 1, fmt, args);
    va_end(args);

    write(STDOUT_FILENO, big, (size_t) len);

    vx_free(big);
    atomic_flag_clear(&g_io_atomic_lock);
}

void vx_warn(const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (fmt == NULL)
    {
        return;
    }

    while (atomic_flag_test_and_set(&g_io_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
    }

    const char *prefix = "\033[38;5;202m[warning]: \033[0m";

    char buf[VX_PATH_MAX];

    i32 prefix_len = snprintf(buf, sizeof(buf), "%s", prefix);

    if (prefix_len < 0 || prefix_len >= (i32) sizeof(buf))
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    va_list args;
    va_start(args, fmt);
    i32 msg_len = vsnprintf(buf + prefix_len, sizeof(buf) - (size_t) prefix_len, fmt, args);
    va_end(args);

    if (msg_len < 0)
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    i32 total_len = prefix_len + msg_len;

    i32 fd = STDERR_FILENO;

    if (total_len < (i32) sizeof(buf))
    {
        buf[total_len] = '\n';
        write(fd, buf, (size_t) total_len + 1);
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    // fallback if large
    char *big = vx_malloc((size_t) total_len + 1);

    if (big == NULL)
    {
        const char *oom = "[warning]: out of memory while formatting warning msg\n";
        write(fd, oom, strlen(oom));
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    memcpy(big, prefix, (size_t) prefix_len);

    va_start(args, fmt);
    vsnprintf(big + prefix_len, (size_t) msg_len + 1, fmt, args);
    va_end(args);

    big[total_len] = '\n';
    write(fd, big, (size_t) total_len + 1);

    vx_free(big);
    atomic_flag_clear(&g_io_atomic_lock);
}

void vx_errlog(const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (fmt == NULL)
    {
        return;
    }

    while (atomic_flag_test_and_set(&g_io_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
    }

    const char *prefix = "\033[38;5;160m[error]: \033[0m";

    char buf[VX_PATH_MAX];

    i32 prefix_len = snprintf(buf, sizeof(buf), "%s", prefix);

    if (prefix_len < 0 || prefix_len >= (i32) sizeof(buf))
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    va_list args;
    va_start(args, fmt);
    i32 msg_len = vsnprintf(buf + prefix_len, sizeof(buf) - (size_t) prefix_len, fmt, args);
    va_end(args);

    if (msg_len < 0)
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    i32 total_len = prefix_len + msg_len;

    i32 fd = STDERR_FILENO;

    if (total_len < (i32) sizeof(buf))
    {
        buf[total_len] = '\n';
        write(fd, buf, (size_t) total_len + 1);
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    // fallback if large
    char *big = vx_malloc((size_t) total_len + 1);

    if (big == NULL)
    {
        const char *oom = "[error]: out of memory while formatting error\n";
        write(fd, oom, strlen(oom));
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    memcpy(big, prefix, (size_t) prefix_len);

    va_start(args, fmt);
    vsnprintf(big + prefix_len, (size_t) msg_len + 1, fmt, args);
    va_end(args);

    big[total_len] = '\n';
    write(fd, big, (size_t) total_len + 1);

    vx_free(big);
    atomic_flag_clear(&g_io_atomic_lock);
}

void vx_log(const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (fmt == NULL)
    {
        return;
    }

    while (atomic_flag_test_and_set(&g_io_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
    }

    const char *prefix = "\033[38;5;40m[log]: \033[0m";

    char buf[VX_PATH_MAX];

    i32 prefix_len = snprintf(buf, sizeof(buf), "%s", prefix);

    if (prefix_len < 0 || prefix_len >= (i32) sizeof(buf))
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    va_list args;
    va_start(args, fmt);
    i32 msg_len = vsnprintf(buf + prefix_len, sizeof(buf) - (size_t) prefix_len, fmt, args);
    va_end(args);

    if (msg_len < 0)
    {
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    i32 total_len = prefix_len + msg_len;

    i32 fd = STDERR_FILENO;

    if (total_len < (i32) sizeof(buf))
    {
        buf[total_len] = '\n';
        write(fd, buf, (size_t) total_len + 1);

        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    // fallback if large
    char *big = vx_malloc((size_t) total_len + 1);

    if (big == NULL)
    {
        const char *oom = "[log]: out of memory while formatting debug log\n";
        write(fd, oom, strlen(oom));
        atomic_flag_clear(&g_io_atomic_lock);
        return;
    }

    memcpy(big, prefix, (size_t) prefix_len);

    va_start(args, fmt);
    vsnprintf(big + prefix_len, (size_t) msg_len + 1, fmt, args);
    va_end(args);

    big[total_len] = '\n';
    write(fd, big, (size_t) total_len + 1);

    vx_free(big);
    atomic_flag_clear(&g_io_atomic_lock);
}

static bool g_vx_set_debug = false;

void vx_set_debug(bool enabled)
{
    g_vx_set_debug = enabled;
}

void vx_dbglog(const char *fmt, ...)
{
    if (!vx_initialized())
    {
        return;
    }

    if (g_vx_set_debug)
    {
        if (fmt == NULL)
        {
            return;
        }

        while (atomic_flag_test_and_set(&g_io_atomic_lock))
        {
            nanosleep(&g_io_ts, NULL);
        }

        const char *prefix = "\033[38;5;167m[debug]: \033[0m";

        char buf[VX_PATH_MAX];

        i32 prefix_len = snprintf(buf, sizeof(buf), "%s", prefix);

        if (prefix_len < 0 || prefix_len >= (i32) sizeof(buf))
        {
            atomic_flag_clear(&g_io_atomic_lock);
            return;
        }

        va_list args;
        va_start(args, fmt);
        i32 msg_len = vsnprintf(buf + prefix_len, sizeof(buf) - (size_t) prefix_len, fmt, args);
        va_end(args);

        if (msg_len < 0)
        {
            atomic_flag_clear(&g_io_atomic_lock);
            return;
        }

        i32 total_len = prefix_len + msg_len;

        i32 fd = STDERR_FILENO;

        if (total_len < (i32) sizeof(buf))
        {
            buf[total_len] = '\n';
            write(fd, buf, (size_t) total_len + 1);

            atomic_flag_clear(&g_io_atomic_lock);
            return;
        }

        // fallback if large
        char *big = vx_malloc((size_t) total_len + 1);

        if (big == NULL)
        {
            const char *oom = "[debug]: out of memory while formatting debug log\n";
            write(fd, oom, strlen(oom));
            atomic_flag_clear(&g_io_atomic_lock);
            return;
        }

        memcpy(big, prefix, (size_t) prefix_len);

        va_start(args, fmt);
        vsnprintf(big + prefix_len, (size_t) msg_len + 1, fmt, args);
        va_end(args);

        big[total_len] = '\n';
        write(fd, big, (size_t) total_len + 1);

        vx_free(big);
        atomic_flag_clear(&g_io_atomic_lock);
    }
}

//----------------------------------------------------------------------------------------------------

vx_status vx_fwrite(const char *path, const char *fmt, ...)
{
    while (atomic_flag_test_and_set(&g_fwrite_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
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
    while (atomic_flag_test_and_set(&g_fwrite_atomic_lock))
    {
        nanosleep(&g_io_ts, NULL);
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
