#include "vx_thread.h"

#if defined(VX_OS_WINDOWS) && defined(VX_USE_THREADS)

static DWORD WINAPI vx_thread_entry(void *arg)
{
    struct vx_thread *t = (struct vx_thread *) arg;

    atomic_store(&t->running, true);

    t->fn(t->arg);

    atomic_store(&t->running, false);

    return 0;
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

    t->handle = CreateThread(nullptr, 0, vx_thread_entry, t, 0, nullptr);

    if (t->handle == nullptr)
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

    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);

    t->handle = nullptr;
    return VX_OK;
}

void vx_thread_detach(struct vx_thread *t)
{
    if (t == nullptr)
    {
        return;
    }

    CloseHandle(t->handle);
    t->handle = nullptr;
}

u64 vx_thread_id(void)
{
    return (u64) GetCurrentThreadId();
}

void vx_mutex_init(vx_mutex *m)
{
    InitializeCriticalSection(m);
}
void vx_mutex_destroy(vx_mutex *m)
{
    DeleteCriticalSection(m);
}
void vx_mutex_lock(vx_mutex *m)
{
    EnterCriticalSection(m);
}
void vx_mutex_unlock(vx_mutex *m)
{
    LeaveCriticalSection(m);
}

void vx_cond_init(vx_cond *c)
{
    InitializeConditionVariable(c);
}
void vx_cond_destroy(vx_cond *c)
{ /* Windows CVs don't need explicit destruction */
}
void vx_cond_signal(vx_cond *c)
{
    WakeConditionVariable(c);
}
void vx_cond_broadcast(vx_cond *c)
{
    WakeAllConditionVariable(c);
}

void vx_cond_wait(vx_cond *c, vx_mutex *m)
{
    SleepConditionVariableCS(c, m, INFINITE);
}

#endif
