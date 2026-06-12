#include "vx_thread.h"

#if defined(VX_USE_THREADS)

    #if defined(VX_OS_WINDOWS)

i32 vx_sem_init(vx_sem *sem, u32 initial_count)
{
    *sem = CreateSemaphore(NULL, initial_count, 0x7FFFFFFF, NULL);
    return (*sem != NULL) ? 0 : -1;
}

void vx_sem_wait(vx_sem *sem)
{
    WaitForSingleObject(*sem, INFINITE);
}

void vx_sem_post(vx_sem *sem)
{
    ReleaseSemaphore(*sem, 1, NULL);
}

void vx_sem_destroy(vx_sem *sem)
{
    CloseHandle(*sem);
}

    #elif defined(VX_OS_MACOS)
i32 vx_sem_init(vx_sem *sem, u32 initial_count)
{
    static atomic_uint counter = 0;

    u32 id = atomic_fetch_add(&counter, 1);

    char name[32];
    snprintf(name, sizeof(name), "/vx_sem_%u", id);
    *sem = sem_open(name, O_CREAT | O_EXCL, 0600, initial_count);
    if (*sem == SEM_FAILED)
    {
        return -1;
    }
    sem_unlink(name);  // unlink immediately, stays alive until sem_close
    return 0;
}
void vx_sem_wait(vx_sem *sem)
{
    while (sem_wait(*sem) == -1 && errno == EINTR)
        ;
}
void vx_sem_post(vx_sem *sem)
{
    sem_post(*sem);
}
void vx_sem_destroy(vx_sem *sem)
{
    sem_close(*sem);
}

    #else

        #include <errno.h>

i32 vx_sem_init(vx_sem *sem, u32 initial_count)
{
    if (sem_init(sem, 0, initial_count) == -1)
    {
        return -1;
    }
    return 0;
}

void vx_sem_wait(vx_sem *sem)
{
    while (sem_wait(sem) == -1 && errno == EINTR)
        ;
}

void vx_sem_post(vx_sem *sem)
{
    sem_post(sem);
}

void vx_sem_destroy(vx_sem *sem)
{
    sem_destroy(sem);
}

    #endif  // PLATFORM

#endif  // VX_USE_THREADS
