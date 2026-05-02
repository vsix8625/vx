#include "vx_time.h"
#include <stdio.h>

// if windows, vx_platform includes <windows.h>
#if defined(VX_OS_MACOS)
#include <mach/mach_time.h>
#elif defined(VX_OS_LINUX)
#include <time.h>
#endif

static u64 g_vx_time_start = 0;

#if defined(VX_OS_WINDOWS)
static uint64_t g_vx_ticks_freq = 0;
#elif defined(VX_OS_MACOS)
static mach_timebase_info_data_t g_vx_timebase_info;
#endif

static u64 vx_get_systemNS(void)
{
#if defined(VX_OS_WINDOWS)
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    return (u64) ((counter.QuadPart * 1000000000ULL) / g_vx_ticks_freq);
#elif defined(VX_OS_MACOS)
    u64 now = mach_absolute_time();
    return (now * g_vx_timebase_info.numer) / g_vx_timebase_info.denom;
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64) ts.tv_sec * 1000000000ULL + ts.tv_nsec;
#endif
}

void vx_init_time(void)
{
#if defined(VX_OS_WINDOWS)
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    g_vx_ticks_freq = (u64) freq.QuadPart;
#elif defined(VX_OS_MACOS)
    mach_timebase_info(&g_vx_timebase_info);
#endif

    g_vx_time_start = vx_get_systemNS();
}

u64 vx_timeNS(void)
{
    return vx_get_systemNS() - g_vx_time_start;
}

u64 vx_time_micro(void)
{
    return vx_timeNS() / 1000ULL;
}

u64 vx_timeMS(void)
{
    return vx_timeNS() / 1000000ULL;
}

f32 vx_timef(void)
{
    // we use double for division for precision
    return (f32) ((f64) vx_timeNS() / 1000000000.0);
}

f64 vx_timef64(void)
{
    return (f64) vx_timeNS() / 1000000000.0;
}

void vx_ticks_start(vx_ticks *t)
{
    t->start = vx_timeNS();
    t->end   = 0;
}

void vx_ticks_end(vx_ticks *t)
{
    t->end = vx_timeNS();
}

char *vx_ticks_format(const vx_ticks *ticks, char *buf, size_t buf_size)
{
    if (ticks == nullptr || buf == nullptr || buf_size == 0)
    {
        snprintf(buf, buf_size, "N/A");
        return buf;
    }

    u64 diff = ticks->end - ticks->start;
    f64 ns   = (f64) diff;

    if (diff < 1000ULL)
    {
        snprintf(buf, buf_size, "%llu ns", (unsigned long long) diff);
    }
    else if (diff < 1000000ULL)
    {
        snprintf(buf, buf_size, "%.2f us", ns / 1000.0);
    }
    else if (diff < 1000000000ULL)
    {
        snprintf(buf, buf_size, "%.2f ms", ns / 1000000.0);
    }
    else if (diff < 60000000000ULL)
    {
        snprintf(buf, buf_size, "%.3f s", ns / 1000000000.0);
    }
    else
    {
        snprintf(buf, buf_size, "%.2f min", ns / 60000000000.0);
    }

    return buf;
}
