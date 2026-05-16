#ifndef VX_PROCESS_H_
#define VX_PROCESS_H_

#include "vx_platform.h"

#if !defined(VX_NO_PROCESS)
    #define VX_USE_PROCESS
#endif

#if defined(VX_USE_PROCESS)

typedef enum sx_proc_flags : u8
{
    VX_PROCESS_FLAGS_NONE = 0,

    VX_PROCESS_FLAGS_BG,
    VX_PROCESS_FLAGS_SILENT,
} vx_proc_flags;

struct vx_proc_cfg
{
    const char *working_dir;
    const char *stdout_path;

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

/**
 * @brief Cross-platform process creation abstraction.
 *
 * Spawns a child process while abstracting the underlying OS mechanism
 * (`posix_spawn` on Linux, CreateProcess on Windows).
 *
 * @param proc Pointer to the `vx_process` handle. This stores the platform-specific
 *             identity of the process (PID/Handle).
 * @param cmd  The command or executable path.
 * @param args A NULL-terminated array of strings.
 *             - On POSIX: Passed directly to `posix_spawnp`.
 *             - On Win32: Flattened into a single command line string, as Win32
 *               treats arguments as a single buffer.
 * @param cfg  Configuration block for fine-grained control:
 *             - cfg->working_dir: Sets the child's starting directory (CWD).
 *             - cfg->flags:
 *                 - `VX_PROCESS_FLAGS_BG`: Detaches the process from the parent's
 *                   session/console.
 *                 - `VX_PROCESS_FLAGS_SILENT`: Discards output (mapped to /dev/null
 *                   on POSIX or suppressed on Win32).
 *
 * @return `vx_status` `VX_OK` if the OS successfully initialized the process,
 *                   `VX_ERROR` otherwise.
 */
VX_API vx_status vx_process_spawn(struct vx_process  *proc,
                                  const char         *cmd,
                                  char              **args,
                                  struct vx_proc_cfg *cfg);

VX_API i32  vx_process_wait(struct vx_process *proc);
VX_API void vx_process_kill(struct vx_process *proc);

#endif

#endif  // VX_PROCESS_H_
