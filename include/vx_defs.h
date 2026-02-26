#ifndef DEFS_H_
#define DEFS_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

#define VX_SV_FMT     "%.*s"
#define VX_SV_ARG(sv) (int) (sv).len, (sv).data

typedef struct vx_string_view
{
    const char *data;
    size_t      len;
} vx_sv;

#define vx_malloc malloc
#define vx_free   free

typedef enum vx_exit : i32
{
    VX_EXIT_SUCCESS = 0,
    VX_EXIT_FAILURE = 1,
} vx_exit;

typedef enum vx_status : i32
{
    VX_OK    = 0,
    VX_ERROR = 1,
    VX_FATAL = 2,
} vx_status;

typedef struct vx_sbuf
{
    char  *data;
    size_t offset;
    size_t size;

    char pad[8];
} vx_sbuf;

#define VX_INTERNAL __attribute__((visibility("hidden")))

#if !defined(__cplusplus)
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
#ifndef nullptr
#define nullptr ((void *) 0)
#endif
#endif
#endif

#endif  // DEFS_H_
