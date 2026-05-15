#include "vx_process.h"

#if defined(VX_OS_LINUX)

    #include <spawn.h>
    #include <sys/wait.h>
    #include <fcntl.h>

extern char **environ;

vx_status
vx_process_spawn(struct vx_process *proc, const char *cmd, char **args, struct vx_proc_cfg *cfg)
{
    if (proc == nullptr || cmd == nullptr || args == nullptr)
    {
        return VX_ERROR;
    }

    posix_spawn_file_actions_t actions;
    posix_spawnattr_t          attr;

    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attr);

    if (cfg != nullptr)
    {
        if (cfg->working_dir)
        {
            posix_spawn_file_actions_addchdir_np(&actions, cfg->working_dir);
        }

        if (cfg->stdout_path)
        {
            posix_spawn_file_actions_addopen(
                &actions, STDOUT_FILENO, cfg->stdout_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (cfg->flags & VX_PROCESS_FLAGS_SILENT)
        {
            if (cfg->stdout_path == nullptr)
            {
                posix_spawn_file_actions_addopen(&actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0);
            }
            posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0);
        }

        if (cfg->flags & VX_PROCESS_FLAGS_BG)
        {
            posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSID);
        }
    }

    i32 status = posix_spawnp(
        &proc->pid, cmd, &actions, &attr, args, (cfg && cfg->envp) ? cfg->envp : environ);

    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);

    if (status != 0)
    {
        return VX_ERROR;
    }

    proc->running = true;

    return VX_OK;
}

i32 vx_process_wait(struct vx_process *proc)
{
    if (proc == nullptr || !proc->running)
    {
        return -1;
    }

    i32 status;
    waitpid(proc->pid, &status, 0);
    proc->exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    proc->running = false;
    return proc->exit_code;
}

void vx_process_kill(struct vx_process *proc)
{
    if (proc != nullptr && proc->running)
    {
        kill(proc->pid, SIGKILL);
        proc->running = false;
    }
}

#endif
