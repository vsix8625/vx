#ifndef VX_THREAD_H_
#define VX_THREAD_H_

#include "vx_platform.h"
#include "vx_defs.h"

#if defined(VX_USE_THREADS)

    #if defined(VX_OS_WINDOWS)
typedef HANDLE vx_sem;
    #else
        #include <semaphore.h>
typedef sem_t vx_sem;
    #endif

VX_API i32  vx_sem_init(vx_sem *sem, u32 initial_count);
VX_API void vx_sem_wait(vx_sem *sem);
VX_API void vx_sem_post(vx_sem *sem);
VX_API void vx_sem_destroy(vx_sem *sem);

#endif  // VX_USE_THREADS
#endif  // VX_THREAD_H_
