#ifndef VX_ARRAY_H_
#define VX_ARRAY_H_

#include "vx_defs.h"
#include "vx_platform.h"

typedef struct vx_array
{
    void **elements;

    size_t count;
    size_t capacity;
} vx_array;

/*
 * Allocates and initializes a dynamic array with initial capacity of `n` elements.
 * Elements are stored as pointers (`void *`).
 * If `initial_capacity` is 0, a default size of 32 is used.
 */
VX_API vx_array *vx_array_create(size_t initial_capacity);

/*
 * Free the dynamic array and its internal pointer.
 * Does nothing if `arr` is `nullptr`.
 */
VX_API void vx_array_destroy(vx_array *arr);

/*
 * Adds an element to the end of the array.
 * Automatically doubles the capacity if the array is full.
 * Does nothing if `arr` is `nullptr` or if reallocation fails.
 */
VX_API void vx_array_push(vx_array *arr, void *element);

/*
 * Removes and returns the last element of the array.
 * Returns `nullptr` if the array is empty or `nullptr`.
 */
VX_API void *vx_array_pop(vx_array *arr);

/*
 * Removes and returns the element at `index`.
 * `NOTE:` This operation does NOT preserve array order.
 * Returns `nullptr` if `index` is out of bounds.
 */
VX_API void *vx_array_pop_at(vx_array *arr, size_t index);

#endif  // VX_ARRAY_H_
