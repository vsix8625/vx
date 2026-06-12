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
#define VX_SV_ARG(sv) (int32_t) (sv).len, (sv).data

typedef struct vx_string_view
{
    const char *data;
    size_t      len;
} vx_sv;

#define VX_SV(s) {s, sizeof(s) - 1}

#ifndef vx_malloc
    #define vx_malloc malloc
#endif

#ifndef vx_calloc
    #define vx_calloc calloc
#endif

#ifndef vx_realloc
    #define vx_realloc realloc
#endif

#ifndef vx_strdup
    #define vx_strdup strdup
#endif

#ifndef vx_free
    #define vx_free free
#endif

typedef enum vx_exit : i32
{
    VX_EXIT_SUCCESS = 0,
    VX_EXIT_FAILURE = 1,
} vx_exit;

typedef enum vx_status : i32
{
    VX_OK = 0,

    VX_ERROR,
    VX_FATAL,

    VX_LIB_NOT_INITIALIZED,
} vx_status;

typedef struct vx_sbuf
{
    char  *data;
    size_t offset;
    size_t size;

    char pad[8];
} vx_sbuf;

#if defined(__GNUC__) || defined(__clang__)
    #define VX_INTERNAL __attribute__((visibility("hidden")))
#elif defined(_MSC_VER)
    #define VX_INTERNAL
#else
    #define VX_INTERNAL
#endif

#if !defined(__cplusplus)
    #if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
        #ifndef nullptr
            #define nullptr ((void *) 0)
        #endif
    #endif
#endif

typedef struct vx_ticks
{
    u64 start;
    u64 end;
} vx_ticks;

typedef void *(*vx_alloc_fn)(void *user, size_t size);

static_assert(sizeof(void *) == 8);

typedef struct
{
    const char *name;
    size_t      name_len;
    bool        is_dir;
} vx_dir_entry;

typedef void *vx_dir_handle;

#define VX_KiB(x) ((u64) (x) << 10)
#define VX_MiB(x) ((u64) (x) << 20)
#define VX_GiB(x) ((u64) (x) << 30)

typedef enum
{
    VX_LOG_LEVEL_PRINTF,
    VX_LOG_LEVEL_INFO,
    VX_LOG_LEVEL_WARN,
    VX_LOG_LEVEL_ERROR,
    VX_LOG_LEVEL_DEBUG,
} vx_log_type;

typedef enum
{
    VX_COLOR_NONE,
    VX_COLOR_GREEN,
    VX_COLOR_ORANGE,
    VX_COLOR_RED,
    VX_COLOR_CORAL,    // 167 (debug color)
    VX_COLOR_BLUE,     // 75  (Vibrant cornflower blue)
    VX_COLOR_MAGENTA,  // 13  (Bright magenta)
} vx_color;

typedef struct vx_vec2
{
    i32 x, y;
} vx_vec2;

typedef struct vx_vec2f
{
    f32 x, y;
} vx_vec2f;

typedef struct vx_vec4f
{
    union
    {
        struct
        {
            f32 x, y, z, w;
        };
        struct
        {
            f32 r, g, b, a;
        };
    };
} vx_vec4f;

#endif  // DEFS_H_
