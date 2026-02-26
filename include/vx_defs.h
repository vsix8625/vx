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

typedef struct vx_string
{
    const char *data;
    size_t      len;
} string_view;

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

#endif  // DEFS_H_
