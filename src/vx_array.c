#include "vx_array.h"
#include "vx_limits.h"
#include "vx_io.h"

#include <stdlib.h>

#define VX_MAX_ARRAY (VX_SIZE_MAX / (2 * sizeof(void *)))

vx_array *vx_array_create(size_t initial_capacity)
{
    if (initial_capacity > VX_MAX_ARRAY)
    {
        VX_ASSERT_LOG("Initial capacity %zu is too large", initial_capacity);
        return nullptr;
    }

    if (initial_capacity == 0)
    {
        initial_capacity = VX_BUF_SIZE_32;
    }

    vx_array *arr = vx_calloc(1, sizeof(vx_array));

    if (arr == nullptr)
    {
        VX_ASSERT_LOG("Failed to allocate %zu bytes for array", initial_capacity);
        return nullptr;
    }

    arr->elements = vx_calloc(initial_capacity, sizeof(void *));

    if (arr->elements == nullptr)
    {
        VX_ASSERT_LOG("Failed to allocate %zu bytes for array elements",
                      initial_capacity * sizeof(void *));
        vx_free(arr);
        return nullptr;
    }

    // arr->count    = 0; // from calloc
    arr->capacity = initial_capacity;

    return arr;
}

void vx_array_destroy(vx_array *arr)
{
    if (arr == nullptr)
    {
        return;
    }

    vx_free(arr->elements);
    vx_free(arr);
}

void vx_array_push(vx_array *arr, void *element)
{
    if (arr == nullptr)
    {
        return;
    }

    if (arr->count >= arr->capacity)
    {
        if (arr->capacity > (VX_MAX_ARRAY))
        {
            VX_ASSERT_LOG("Array reached absolute maximum capacity");
            return;
        }

        size_t new_cap  = arr->capacity << 1;
        size_t new_size = sizeof(void *) * new_cap;

        void **tmp = vx_realloc(arr->elements, new_size);

        if (tmp == nullptr)
        {
            VX_ASSERT_LOG("Failed to reallocate dynamic array to %zu", new_cap);
            return;
        }

        arr->elements = tmp;
        arr->capacity = new_cap;
    }

    arr->elements[arr->count++] = element;
}

void *vx_array_pop_at(vx_array *arr, size_t index)
{
    if (arr == nullptr || index >= arr->count || arr->count == 0)
    {
        return nullptr;
    }

    void *element = arr->elements[index];

    // swap with last element
    arr->elements[index] = arr->elements[arr->count - 1];

    arr->elements[arr->count - 1] = nullptr;
    arr->count--;

    return element;
}

void *vx_array_pop(vx_array *arr)
{
    if (arr == nullptr || arr->count == 0)
    {
        return nullptr;
    }

    return vx_array_pop_at(arr, arr->count - 1);
}
