#ifndef VX_H_
#define VX_H_

#include "vx_io.h"
#include "vx_string.h"
#include "vx_array.h"
#include "vx_config.h"
#include "vx_time.h"

/*
 * @return: `true` if initialized or `false`.
 */
bool vx_initialized(void);

/*
 * Initializes vx library.
 * @return: `VX_OK` on success, `VX_ERROR` if failed.*/
vx_status vx_init(void);

/*
 *Shutdown vx library
 */
void vx_shutdown(void);

#endif  // VX_H_
