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

u32 vx_cpu_get_cache_line(void)
{
#if defined(VX_OS_WINDOWS)
    size_t cache_line_size = 64;
    DWORD  buffer_size     = 0;

    if (!GetLogicalProcessorInformation(NULL, &buffer_size) &&
        GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    {
        HANDLE                                heap = GetProcessHeap();
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer =
            (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION) HeapAlloc(heap, HEAP_ZERO_MEMORY, buffer_size);

        if (buffer)
        {
            if (GetLogicalProcessorInformation(buffer, &buffer_size))
            {
                DWORD element_count = buffer_size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

                for (DWORD i = 0; i < element_count; i++)
                {
                    if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1)
                    {
                        cache_line_size = buffer[i].Cache.LineSize;
                        break;
                    }
                }
            }
            HeapFree(heap, 0, buffer);
        }
    }

    return (u32) cache_line_size;
#else
    i64 size = sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
    return (size <= 0) ? 64 : (u32) size;
#endif
}
