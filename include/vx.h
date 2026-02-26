#ifndef VX_H_
#define VX_H_

#include "vx_io.h"
#include "vx_string.h"

bool vx_initialized(void);

vx_status vx_init(void);
vx_status vx_shutdown(void);

#endif  // VX_H_
