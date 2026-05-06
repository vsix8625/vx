#ifndef VX_H_
#define VX_H_

#include "vx_io.h"
#include "vx_string.h"
#include "vx_array.h"
#include "vx_config.h"
#include "vx_time.h"
#include "vx_util.h"
#include "vx_fs.h"

/*
 * @return: `true` if initialized or `false`.
 */
VX_API bool vx_initialized(void);

/*
 * Initializes vx library.
 * @return: `VX_OK` on success, `VX_ERROR` if failed.*/
VX_API vx_status vx_init(void);

/*
 *Shutdown vx library
 */
VX_API void vx_shutdown(void);

#endif  // VX_H_
