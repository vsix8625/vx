#include "vx_io.h"
#include "vx_process.h"

#if defined(VX_OS_LINUX) && defined(VX_USE_PROCESS)

    #include <spawn.h>
    #include <sys/wait.h>
    #include <fcntl.h>

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

    proc->read_pipe_fd = -1;

    i32 write_fd_to_close = -1;

    if (cfg != nullptr)
    {
        if (cfg->working_dir)
        {
            posix_spawn_file_actions_addchdir_np(&actions, cfg->working_dir);
        }

        // TODO: add win32 version of this
        if (cfg->flags & VX_PROCESS_FLAGS_CAPTURE)
        {
            // NOTE: will use fork cause posix pipe fails
            //    i32 pipe_fds[2];
            // if (pipe(pipe_fds) == 0)
            // {
            //     vx_errlog("PIPE CREATED: read=%d write=%d", pipe_fds[0], pipe_fds[1]);
            //     proc->read_pipe_fd = pipe_fds[0];
            //     write_fd_to_close  = pipe_fds[1];
            //
            //     posix_spawn_file_actions_adddup2(&actions, pipe_fds[1], STDERR_FILENO);
            //     posix_spawn_file_actions_adddup2(&actions, pipe_fds[1], STDOUT_FILENO);
            //
            //     posix_spawn_file_actions_addclose(&actions, pipe_fds[0]);
            //     posix_spawn_file_actions_addclose(&actions, pipe_fds[1]);
            // }
            i32 pipe_fds[2];
            if (pipe(pipe_fds) != 0)
            {
                return VX_ERROR;
            }

            pid_t pid = fork();
            if (pid == 0)
            {
                dup2(pipe_fds[1], STDOUT_FILENO);
                dup2(pipe_fds[1], STDERR_FILENO);
                close(pipe_fds[0]);
                close(pipe_fds[1]);
                execvp(cmd, args);
                _exit(1);
            }

            close(pipe_fds[1]);
            proc->read_pipe_fd = pipe_fds[0];
            proc->pid          = pid;
            proc->running      = true;
            posix_spawn_file_actions_destroy(&actions);
            posix_spawnattr_destroy(&attr);
            return VX_OK;
        }
        else if (cfg->stdout_path)
        {
            posix_spawn_file_actions_addopen(
                &actions, STDOUT_FILENO, cfg->stdout_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        }

        if (cfg->flags & VX_PROCESS_FLAGS_SILENT)
        {
            if (cfg->stdout_path == nullptr)
            {
                posix_spawn_file_actions_addopen(
                    &actions, STDOUT_FILENO, "/dev/null", O_WRONLY, 0644);
            }
            posix_spawn_file_actions_addopen(&actions, STDERR_FILENO, "/dev/null", O_WRONLY, 0644);
        }

        if (cfg->flags & VX_PROCESS_FLAGS_BG)
        {
            posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETSID);
        }
    }

    i32 status = posix_spawnp(
        &proc->pid, cmd, &actions, &attr, args, (cfg && cfg->envp) ? cfg->envp : environ);

    if (cfg && (cfg->flags & VX_PROCESS_FLAGS_CAPTURE))
    {
        close(write_fd_to_close);
    }

    posix_spawn_file_actions_destroy(&actions);
    posix_spawnattr_destroy(&attr);

    if (status != 0)
    {
        if (proc->read_pipe_fd != -1)
        {
            close(proc->read_pipe_fd);
            proc->read_pipe_fd = -1;
        }
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

// TODO: add win32 version of this
void vx_process_consume_output(struct vx_process *proc, vx_sbuf *thread_local_buf)
{
    if (proc->read_pipe_fd == -1)
    {
        return;
    }

    char    tmp_chunk[512];
    ssize_t bytes_read;

    while ((bytes_read = read(proc->read_pipe_fd, tmp_chunk, sizeof(tmp_chunk) - 1)) > 0)
    {
        tmp_chunk[bytes_read] = '\0';
        vx_sbuf_append(thread_local_buf, "%s", tmp_chunk);
    }

    close(proc->read_pipe_fd);
    proc->read_pipe_fd = -1;
}

#endif
