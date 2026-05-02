#ifndef VX_PLATFORM_H_
#define VX_PLATFORM_H_

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <direct.h>
    #include <windows.h>
    #define vx_write       _write
    #define vx_getcwd      _getcwd
    #define vx_mkdir(dir)  _mkdir((dir))
    #define vx_isatty      _isatty
    #define vx_stat        _stat
    #define vx_stat_struct struct _stat

    #define STDOUT_FILENO 1
    #define STDERR_FILENO 2
    #define VX_FILENO     _fileno
    #define VX_DEVNUL     "nul"

    #define VX_PATH_SEP     '\\'
    #define VX_PATH_SEP_STR "\\"

    #ifndef S_ISDIR
        #define S_ISDIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
    #endif

#else

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif

    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>

    #define vx_write        write
    #define vx_getcwd       getcwd
    #define vx_mkdir(dir)   mkdir((dir), 0755)
    #define vx_stat         stat
    #define vx_stat_struct  struct stat
    #define vx_isatty       isatty
    #define VX_PATH_SEP     '/'
    #define VX_PATH_SEP_STR "/"

    #define VX_FILENO fileno
    #define VX_DEVNUL "/dev/null"
#endif

#if defined(_WIN32) || defined(_WIN64)
    #define VX_OS_WINDOWS 1
    #define VX_PATH_MAX   (512)
#elif defined(__APPLE__) || defined(__MACH__)
    #define VX_OS_MACOS 1
    #define VX_PATH_MAX (1024)
#elif defined(__linux__) || defined(__linux)
    #define VX_OS_LINUX 1
    #define VX_PATH_MAX (4096)
#endif

#ifndef VX_API
    #if defined(VX_OS_WINDOWS)
        #ifdef VX_BUILD_DLL
            #define VX_API __declspec(dllexport)
        #else
            #define VX_API __declspec(dllimport)
        #endif
    #else
        #if __GNUC__ >= 4
            #define VX_API __attribute__((visibility("default")))
        #else
            #define VX_API
        #endif
    #endif
#endif

VX_API void vx_io_init(void);

#endif  // VX_PLATFORM_H_
