#include "vx_process.h"
#include "vx_util.h"

// NOTE: windows code untested

#if defined(VX_OS_WINDOWS) && defined(VX_USE_PROCESS)

vx_status
vx_process_spawn(struct vx_process *proc, const char *cmd, char **args, struct vx_proc_cfg *cfg)
{
    if (proc == nullptr || cmd == nullptr)
    {
        return VX_ERROR;
    }

    STARTUPINFO         si = {0};
    PROCESS_INFORMATION pi = {0};

    si.cb = sizeof(si);

    char cmd_line[VX_CMD_LINE_MAX] = {0};

    size_t offset = 0;

    for (i32 i = 0; args[i]; i++)
    {
        size_t len = strlen(args[i]);

        if (offset + len + 2 >= sizeof(cmd_line))
        {
            break;
        }

        memcpy(cmd_line + offset, args[i], len);
        offset             += len;
        cmd_line[offset++]  = ' ';
    }
    cmd_line[offset] = '\0';

    DWORD flags = 0;

    if (cfg)
    {
        // TODO: can it handle cfg->working_dir?

        if (cfg->flags & VX_PROCESS_FLGAGS_BG)
        {
            flags |= DETACHED_PROCESS;
        }

        if (cfg->stdout_path)
        {
            HANDLE hFile   = CreateFileA(cfg->stdout_path,
                                         GENERIC_WRITE,
                                         FILE_SHARE_READ,
                                         nullptr,
                                         CREATE_ALWAYS,
                                         FILE_ATTRIBUTE_NORMAL,
                                         nullptr);
            si.hStdOutput  = hFile;
            si.dwFlags    |= STARTF_USESTDHANDLES;
        }
    }

    // NOTE: for now processA
    if (!CreateProcessA(NULL,
                        cmd_line,
                        nullptr,
                        nullptr,
                        FALSE,
                        flags,
                        nullptr,
                        cfg ? cfg->working_dir : nullptr,
                        &si,
                        &pi))
    {
        return VX_ERROR;
    }

    proc->handle        = pi.hProcess;
    proc->thread_handle = pi.hThread;
    proc->running       = true;

    return VX_OK;
}

i32 vx_process_wait(struct vx_process *proc)
{
    if (proc == nullptr || !proc->running)
    {
        return -1;
    }

    WaitForSingleObject(proc->handle, INFINITE);

    DWORD exit_code;
    GetExitCodeProcess(proc->handle, &exit_code);

    CloseHandle(proc->handle);
    CloseHandle(proc->thread_handle);

    proc->running   = false;
    proc->exit_code = (i32) exit_code;

    return proc->exit_code;
}

void vx_process_kill(struct vx_process *proc)
{
    if (proc && proc->handle)
    {
        TerminateProcess(proc->handle, 1);
        CloseHandle(proc->handle);
        CloseHandle(proc->thread_handle);
        proc->running = false;
    }
}

#endif
