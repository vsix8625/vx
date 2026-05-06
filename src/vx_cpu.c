#include "vx_platform.h"
#include "vx_cpu.h"

u32 vx_cpu_get_nproc(void)
{
    i64 nproc = 1;

#if defined(VX_OS_WINDOWS)
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    nproc = sysinfo.dwNumberOfProcessors;
#else
    nproc = sysconf(_SC_NPROCESSORS_ONLN);
#endif

    if (nproc < 1)
    {
        nproc = 1;
    }

    return (u32) nproc;
}
