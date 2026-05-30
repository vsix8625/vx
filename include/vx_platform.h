#ifndef VX_PLATFORM_H_
#define VX_PLATFORM_H_

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <direct.h>
    #include <windows.h>
    #include "vx_defs.h"

    #define vx_write       _write
    #define vx_getcwd      _getcwd
    #define vx_mkdir(dir)  _mkdir((dir))
    #define vx_isatty      _isatty
    #define vx_stat        _stat64
    #define vx_stat_struct struct __stat64
    #define vx_chdir       _chdir
    #define vx_access      _access
    #define VX_F_OK        0
    #define VX_X_OK        0

    #define STDOUT_FILENO 1
    #define STDERR_FILENO 2

    #define VX_FILENO _fileno
    #define VX_DEVNUL "nul"

    #define VX_PATH_SEP     '\\'
    #define VX_PATH_SEP_STR "\\"

    // Platform Artifact Extensions
    #define VX_EXE_EXT    ".exe"
    #define VX_LIB_EXT    ".lib"
    #define VX_DLL_EXT    ".dll"
    #define VX_LIB_PREFIX ""

typedef CRITICAL_SECTION   vx_mutex;
typedef CONDITION_VARIABLE vx_cond;

    #ifndef S_ISDIR
        #define S_ISDIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
    #endif

#else

    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif

    #include "vx_defs.h"
    #include <unistd.h>
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <pwd.h>

    #define vx_write       write
    #define vx_getcwd      getcwd
    #define vx_mkdir(dir)  mkdir((dir), 0755)
    #define vx_stat        stat
    #define vx_stat_struct struct stat
    #define vx_chdir       chdir
    #define vx_isatty      isatty
    #define vx_access      access
    #define VX_F_OK        F_OK
    #define VX_X_OK        X_OK

    #define VX_PATH_SEP     '/'
    #define VX_PATH_SEP_STR "/"

    #define VX_FILENO     fileno
    #define VX_DEVNUL     "/dev/null"

    // Platform Artifact Extensions
    #define VX_EXE_EXT    ""
    #define VX_LIB_EXT    ".a"
    #define VX_DLL_EXT    ".so"
    #define VX_LIB_PREFIX "lib"

typedef pthread_mutex_t vx_mutex;
typedef pthread_cond_t  vx_cond;

#endif

#if defined(_WIN32) || defined(_WIN64)
    #define VX_OS_WINDOWS 1
    #define VX_OS_NAME    "win32"
#elif defined(__APPLE__) || defined(__MACH__)
    #define VX_OS_MACOS 1
    #error "macOS not supported"
#elif defined(__linux__) || defined(__linux)
    #define VX_OS_LINUX 1
    #define VX_OS_NAME  "linux"
#else
    #error "Unsupported platform"
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

VX_API const char *vx_platform_get_cache_dir(void);

VX_API const char *vx_platform_get_config_dir(void);

VX_API const char *vx_platform_get_home_dir(void);

VX_API vx_status vx_platform_setenv(const char *name, const char *value);

#endif  // VX_PLATFORM_H_
