#pragma GCC system_header
#ifndef IO_H_
    #define IO_H_

    #include "vx_limits.h"

    #define VA_CHECK(fmt_arg_n) __attribute__((format(__printf__, fmt_arg_n, fmt_arg_n + 1)))

VX_API void vx_printf(const char *fmt, ...) VA_CHECK(1);
VX_API void vx_warn(const char *fmt, ...) VA_CHECK(1);
VX_API void vx_errlog(const char *fmt, ...) VA_CHECK(1);
VX_API void vx_log(const char *fmt, ...) VA_CHECK(1);

/*
 * Set debug output for `vx_dbglog` function.
 * @default: false.
 */
VX_API void vx_set_debug(bool enabled);

VX_API void vx_dbglog(const char *fmt, ...) VA_CHECK(1);

//----------------------------------------------------------------------------------------------------

/* Write formatted output to a file at `path`. Thread-safe via spinlock.
 * @return: `VX_OK` on success, `VX_ERROR` if the file could not be opened or written.
 */
VX_API vx_status vx_fwrite(const char *path, const char *fmt, ...) VA_CHECK(2);

/* Append formatted output to a file at `path`. Thread-safe via spinlock.
 * @return: `VX_OK` on success, `VX_ERROR` if the file could not be opened or written.
 */
VX_API vx_status vx_fappend(const char *path, const char *fmt, ...) VA_CHECK(2);

/*
 *
 * `char buf[VX_PATH_MAX];`
 *
 *  `vx_sbuf` sbuf = {
 *                         .data   = buf,
 *                         .size   = sizeof(buf),
 *                         .offset = 0
 *                       };
 */
VX_API void vx_sbuf_append(struct vx_sbuf *buf, const char *fmt, ...) VA_CHECK(2);

//----------------------------------------------------------------------------------------------------

    #define VX_LOAD_CHECK() vx_printf("Hello, from VX\n")

    #define VX_ASSERT_LOG(msg, ...)                                                                \
        do                                                                                         \
        {                                                                                          \
            vx_errlog("In --> %s:%d(%s)\n          \u2514\u2500\u2500" msg,                        \
                      __FILE__,                                                                    \
                      __LINE__,                                                                    \
                      __func__,                                                                    \
                      ##__VA_ARGS__);                                                              \
        } while (0)

#endif  // IO_H_
