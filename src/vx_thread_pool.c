#include "vx_thread.h"
#include "vx_util.h"

#if defined(VX_USE_THREADS)
    #include <stdlib.h>

//----------------------------------------------------------------------------------------------------

static vx_status vx_job_queue_init(struct vx_job_queue *q, u32 size);
static vx_status vx_job_queue_push(struct vx_job_queue *q, vx_thread_fn fn, void *arg);
static vx_status vx_job_queue_pop(struct vx_job_queue *q, struct vx_job *job);
static void      vx_job_queue_destroy(struct vx_job_queue *q);

//----------------------------------------------------------------------------------------------------

static void *vx_pool_worker(void *arg)
{
    struct vx_thread_pool *pool = (struct vx_thread_pool *) arg;

    while (1)
    {
        vx_sem_wait(&pool->sem);
        if (atomic_load(&pool->shutdown))
        {
            break;
        }

        struct vx_job job = {0};
        if (vx_job_queue_pop(&pool->queue, &job) == VX_OK)
        {
            atomic_fetch_add(&pool->active_jobs, 1);

            job.fn(job.arg);

            atomic_fetch_sub(&pool->active_jobs, 1);
        }
    }

    return nullptr;
}

vx_status vx_thread_pool_create(struct vx_thread_pool *pool, u32 thread_count, u32 queue_size)
{
    if (pool == nullptr || thread_count == 0 || queue_size == 0)
    {
        return VX_ERROR;
    }

    pool->thread_count = thread_count;
    atomic_store(&pool->active_jobs, 0);
    atomic_store(&pool->shutdown, false);

    if (vx_job_queue_init(&pool->queue, queue_size) != VX_OK)
    {
        return VX_ERROR;
    }

    if (vx_sem_init(&pool->sem, 0) != VX_OK)
    {
        vx_job_queue_destroy(&pool->queue);
        return VX_ERROR;
    }

    pool->threads = vx_calloc(thread_count, sizeof(struct vx_thread));

    if (pool->threads == nullptr)
    {
        vx_sem_destroy(&pool->sem);
        vx_job_queue_destroy(&pool->queue);
        return VX_ERROR;
    }

    for (u32 i = 0; i < thread_count; i++)
    {
        if (vx_thread_create(&pool->threads[i], vx_pool_worker, pool) != VX_OK)
        {
            atomic_store(&pool->shutdown, true);
            vx_sem_post(&pool->sem);
            vx_sem_destroy(&pool->sem);
            vx_job_queue_destroy(&pool->queue);
            vx_free(pool->threads);
            return VX_ERROR;
        }
    }
    return VX_OK;
}

vx_status vx_thread_pool_push(struct vx_thread_pool *pool, vx_thread_fn fn, void *arg)
{
    if (pool == nullptr || fn == nullptr)
    {
        return VX_ERROR;
    }

    if (atomic_load(&pool->shutdown))
    {
        return VX_ERROR;
    }

    if (vx_job_queue_push(&pool->queue, fn, arg) != VX_OK)
    {
        return VX_ERROR;
    }

    vx_sem_post(&pool->sem);
    return VX_OK;
}

vx_status vx_thread_pool_wait(struct vx_thread_pool *pool)
{
    if (pool == nullptr)
    {
        return VX_ERROR;
    }

    u32 spins = 0;

    while (atomic_load(&pool->active_jobs) > 0 ||
           atomic_load(&pool->queue.head) != atomic_load(&pool->queue.tail))
    {
        if (spins < 1000)
        {
            vx_pause();
            spins++;
        }
        else
        {
            vx_yield();
        }
        vx_pause();
    }
    return VX_OK;
}

void vx_thread_pool_destroy(struct vx_thread_pool *pool)
{
    if (pool == nullptr)
    {
        return;
    }

    atomic_store(&pool->shutdown, true);

    for (u32 i = 0; i < pool->thread_count; i++)
    {
        vx_sem_post(&pool->sem);
    }

    for (u32 i = 0; i < pool->thread_count; i++)
    {
        vx_thread_join(&pool->threads[i]);
    }

    vx_job_queue_destroy(&pool->queue);
    vx_sem_destroy(&pool->sem);

    vx_free(pool->threads);
    pool->threads = nullptr;
}

//----------------------------------------------------------------------------------------------------

static vx_status vx_job_queue_init(struct vx_job_queue *q, u32 size)
{
    if (q == nullptr)
    {
        return VX_ERROR;
    }

    q->jobs = vx_calloc(size, sizeof(struct vx_job));

    if (q->jobs == nullptr)
    {
        return VX_ERROR;
    }

    q->size = size;
    atomic_store(&q->head, 0);
    atomic_store(&q->tail, 0);
    return VX_OK;
}

static vx_status vx_job_queue_push(struct vx_job_queue *q, vx_thread_fn fn, void *arg)
{
    if (q == nullptr || fn == nullptr)
    {
        return VX_ERROR;
    }

    u32 tail = atomic_load(&q->tail);
    u32 next = (tail + 1) % q->size;

    if (next == atomic_load(&q->head))
    {
        return VX_ERROR;  // full
    }

    q->jobs[tail].fn  = fn;
    q->jobs[tail].arg = arg;

    atomic_store_explicit(&q->tail, next, memory_order_release);

    return VX_OK;
}

static vx_status vx_job_queue_pop(struct vx_job_queue *q, struct vx_job *job)
{
    u32 head = atomic_load_explicit(&q->head, memory_order_acquire);

    while (1)
    {
        if (head == atomic_load_explicit(&q->tail, memory_order_acquire))
        {
            return VX_ERROR;
        }

        u32 next = (head + 1) % q->size;

        if (atomic_compare_exchange_weak_explicit(
                &q->head, &head, next, memory_order_acq_rel, memory_order_acquire))
        {
            *job = q->jobs[head];
            return VX_OK;
        }
    }
}

static void vx_job_queue_destroy(struct vx_job_queue *q)
{
    if (q == nullptr)
    {
        return;
    }

    vx_free(q->jobs);
    q->jobs = nullptr;
    q->size = 0;
}

//----------------------------------------------------------------------------------------------------

#endif
