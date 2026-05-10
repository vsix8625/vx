#ifndef VX_PROCESS_H_
#define VX_PROCESS_H_

#include "vx_platform.h"
#include "vx_defs.h"

typedef enum sx_proc_flags : u8
{
    VX_PROCESS_FLAGS_NONE = 0,

    VX_PROCESS_FLAGS_BG,
    VX_PROCESS_FLAGS_SILENT,
} vx_proc_flags;

struct vx_proc_cfg
{
    const char *working_dir;

    char **envp;

    vx_proc_flags flags;
};

struct vx_process
{
#if defined(VX_OS_WINDOWS)
    HANDLE handle;
    HANDLE thread_handle;
#else
    pid_t pid;
#endif

    i32  exit_code;
    bool running;
};

VX_API vx_status vx_process_spawn(struct vx_process  *proc,
                                  const char         *cmd,
                                  char              **args,
                                  struct vx_proc_cfg *cfg);

VX_API i32  vx_process_wait(struct vx_process *proc);
VX_API void vx_process_kill(struct vx_process *proc);

#endif  // VX_PROCESS_H_
