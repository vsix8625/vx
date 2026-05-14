#include "vx_thread.h"

#if defined(VX_OS_LINUX)
    #include <pthread.h>

static void *vx_thread_entry(void *arg)
{
    struct vx_thread *t = (struct vx_thread *) arg;

    atomic_store(&t->running, true);

    t->fn(t->arg);

    atomic_store(&t->running, false);

    return nullptr;
}

vx_status vx_thread_create(struct vx_thread *t, vx_thread_fn fn, void *arg)
{
    if (t == nullptr || fn == nullptr)
    {
        return VX_ERROR;
    }

    t->fn  = fn;
    t->arg = arg;

    atomic_store(&t->running, false);

    if (pthread_create(&t->handle, nullptr, vx_thread_entry, t) != VX_OK)
    {
        return VX_ERROR;
    }

    return VX_OK;
}

vx_status vx_thread_join(struct vx_thread *t)
{
    if (t == nullptr)
    {
        return VX_ERROR;
    }

    pthread_join(t->handle, nullptr);
    return VX_OK;
}

void vx_thread_detach(struct vx_thread *t)
{
    if (t == nullptr)
    {
        return;
    }

    pthread_detach(t->handle);
}

u64 vx_thread_id(void)
{
    return (u64) pthread_self();
}

void vx_mutex_init(vx_mutex *m)
{
    pthread_mutex_init(m, NULL);
}

void vx_mutex_destroy(vx_mutex *m)
{
    pthread_mutex_destroy(m);
}

void vx_mutex_lock(vx_mutex *m)
{
    pthread_mutex_lock(m);
}

void vx_mutex_unlock(vx_mutex *m)
{
    pthread_mutex_unlock(m);
}

void vx_cond_init(vx_cond *c)
{
    pthread_cond_init(c, NULL);
}

void vx_cond_destroy(vx_cond *c)
{
    pthread_cond_destroy(c);
}

void vx_cond_signal(vx_cond *c)
{
    pthread_cond_signal(c);
}

void vx_cond_broadcast(vx_cond *c)
{
    pthread_cond_broadcast(c);
}

void vx_cond_wait(vx_cond *c, vx_mutex *m)
{
    pthread_cond_wait(c, m);
}

#endif
