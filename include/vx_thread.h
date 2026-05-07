#ifndef VX_THREAD_H_
#define VX_THREAD_H_

#include "vx_platform.h"
#include "vx_defs.h"

#if defined(VX_USE_THREADS)

    #include <stdatomic.h>

    #if defined(VX_OS_WINDOWS)
typedef HANDLE vx_sem;
    #else
        #include <semaphore.h>
typedef sem_t vx_sem;
    #endif

typedef void *(*vx_thread_fn)(void *);

struct vx_thread
{
    #if defined(VX_OS_WINDOWS)
    HANDLE handle;
    #else
    pthread_t handle;
    #endif

    vx_thread_fn fn;
    void        *arg;
    atomic_bool  running;
};

struct vx_job
{
    vx_thread_fn fn;
    void        *arg;
};

struct vx_job_queue
{
    struct vx_job *jobs;

    u32 size;

    atomic_uint head;
    atomic_uint tail;
};

struct vx_thread_pool
{
    struct vx_thread   *threads;
    struct vx_job_queue queue;

    u32 thread_count;

    atomic_uint active_jobs;

    vx_sem sem;

    atomic_bool shutdown;
};

// clang-format off

VX_API vx_status vx_thread_create(struct vx_thread *t, vx_thread_fn fn, void *arg);
VX_API vx_status vx_thread_join(struct vx_thread *t);
VX_API void      vx_thread_detach(struct vx_thread *t);
VX_API u64       vx_thread_id(void);

VX_API vx_status vx_thread_pool_create(struct vx_thread_pool *pool, u32 thread_count, u32 queue_size);
VX_API vx_status vx_thread_pool_push(struct vx_thread_pool *pool, vx_thread_fn fn, void *arg);
VX_API vx_status vx_thread_pool_wait(struct vx_thread_pool *pool);
VX_API void      vx_thread_pool_destroy(struct vx_thread_pool *pool);

VX_API i32  vx_sem_init(vx_sem *sem, u32 initial_count);
VX_API void vx_sem_wait(vx_sem *sem);
VX_API void vx_sem_post(vx_sem *sem);
VX_API void vx_sem_destroy(vx_sem *sem);

#endif  // VX_USE_THREADS
#endif  // VX_THREAD_H_
