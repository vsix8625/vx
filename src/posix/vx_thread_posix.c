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

#endif
