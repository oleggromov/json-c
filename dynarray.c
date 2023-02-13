#include <stdlib.h>

#include "dynarray.h"
#include "util.h"

const size_t SIZE_INC = 64;

dynarray_t* dynarray_create(void)
{
  dynarray_t* arr = malloc(sizeof(dynarray_t));
  arr->_arr = NULL;
  arr->_capacity = 0;
  arr->len = 0;
  return arr;
}

void dynarray_free(dynarray_t** arr)
{
  free((*arr)->_arr);
  free(*arr);
  arr = NULL;
}

static void _assert_index(dynarray_t* arr, size_t index)
{
  if (index > arr->_capacity - 1 || index < 0) {
    die("dynarray: index out of bounds");
  }
}

// Returns
// - NULL if value did not replace another one
// - void* to a previous value if replaced
void* dynarray_set(dynarray_t* arr, size_t index, void* value)
{
  // grow if needed
  if (arr->len + 1 > arr->_capacity) {
    arr->_capacity += SIZE_INC;
    arr->_arr = realloc(arr->_arr, sizeof(void*) * arr->_capacity);
    if (arr->_arr == NULL) {
      die("dynarray: couldn't reallocate memory");
    }
  }

  void* old_value_ptr = dynarray_get(arr, index);
  arr->_arr[index] = value;
  arr->len += 1;
  return old_value_ptr;
}

void* dynarray_get(dynarray_t* arr, size_t index)
{
  _assert_index(arr, index);
  return arr->_arr[index];
}

void dynarray_append(dynarray_t* arr, void* value)
{
  dynarray_set(arr, arr->len, value);
}

void* dynarray_get_top(dynarray_t* arr)
{
  return dynarray_get(arr, arr->len - 1);
}

void* dynarray_remove_top(dynarray_t* arr)
{
  if (arr->len == 0) {
    die("dynarray: length is 0, nothing to pop");
  }

  void* value_ptr = arr->_arr[arr->len - 1];
  arr->_arr[arr->len - 1] = NULL;
  arr->len--;
  return value_ptr;
}
