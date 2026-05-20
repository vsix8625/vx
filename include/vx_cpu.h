#ifndef VX_CPU_H_
#define VX_CPU_H_

#include "vx_platform.h"

VX_API u32 vx_cpu_get_nproc(void);
VX_API u32 vx_cpu_get_cache_line(void);

#if defined(__x86_64__) || defined(_M_X64)
    #define VX_ARCH_NAME "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define VX_ARCH_NAME "arm64"
#elif defined(__i386__) || defined(_M_IX86)
    #define VX_ARCH_NAME "x86"
#elif defined(__arm__)
    #define VX_ARCH_NAME "arm"
#else
    #define VX_ARCH_NAME "unknown"
#endif

static inline bool vx_cpu_has_avx(void)
{
#if defined(VX_OS_WINDOWS)
    i32 info[4];
    __cpuidex(info, 1, 0);
    return (info[2] & (1 << 28)) != 0;  // ECX bit 28
#else
    return __builtin_cpu_supports("avx");
#endif
}

static inline bool vx_cpu_has_avx2(void)
{
#if defined(VX_OS_WINDOWS)
    i32 info[4];
    __cpuidex(info, 7, 0);
    return (info[1] & (1 << 5)) != 0;  // EBX bit 5
#else
    return __builtin_cpu_supports("avx2");
#endif
}

static inline bool vx_cpu_has_sse4_2(void)
{
#if defined(VX_OS_WINDOWS)
    i32 info[4];
    __cpuidex(info, 1, 0);
    return (info[2] & (1 << 20)) != 0;  // ECX bit 20
#else
    return __builtin_cpu_supports("sse4.2");
#endif
}

static inline bool vx_cpu_has_bmi(void)
{
#if defined(VX_OS_WINDOWS)
    i32 info[4];
    __cpuidex(info, 7, 0);
    return (info[1] & (1 << 3)) != 0;  // EBX bit 3
#else
    return __builtin_cpu_supports("bmi");
#endif
}

#endif  // VX_CPU_H_
