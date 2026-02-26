#pragma GCC system_header
#ifndef VX_STRING_H_
#define VX_STRING_H_

#include <string.h>

static inline bool vx_strncmplit(const char *s, size_t len, const char *lit, size_t lit_len)
{
    if (len != lit_len)
    {
        return false;
    }

    return memcmp(s, lit, lit_len) == 0;
}

#endif  // VX_STRING_H_
