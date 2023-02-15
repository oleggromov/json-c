#include <stdlib.h>
#include <string.h>

#include "dynarray.h"
#include "util.h"

#include <stdio.h>

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
    die("dynarray: index %d out of bounds (capacity=%d)", index, arr->_capacity);
  }
}

static void _grow_if_needed(dynarray_t* arr)
{
  if (arr->len + 1 > arr->_capacity) {
    size_t old_size = arr->_capacity;

    arr->_capacity += SIZE_INC;
    arr->_arr = realloc(arr->_arr, sizeof(void*) * arr->_capacity);
    memset(arr->_arr + old_size, 0, sizeof(void*) * SIZE_INC); // important for sparse arrays

    if (arr->_arr == NULL) {
      die("dynarray: couldn't reallocate memory");
    }
  }
}

// Returns
// - NULL if value did not replace another one
// - void* to a previous value if replaced
void* dynarray_set(dynarray_t* arr, size_t index, void* value)
{
  _grow_if_needed(arr);

  void* old_value_ptr = dynarray_get(arr, index);
  arr->_arr[index] = value;
  arr->len = index > arr->len ? index + 1: arr->len + 1; // when array is sparse
  return old_value_ptr;
}

void* dynarray_get(dynarray_t* arr, size_t index)
{
  _assert_index(arr, index);
  return arr->_arr[index];
}

// Cuts the value at index from array1
// Returns a pointer to the deleted item (may be void*)
void* dynarray_cut(dynarray_t* arr, size_t index)
{
  void *old_value_ptr = dynarray_get(arr, index);

  // index = 1, len = 5
  // {1, 2, 3, 4, 5} => // {1, 3, 4, 5}
  size_t move_items = arr->len - index; // should be -1 but +1 item is to ensure zeroing
  memmove(
    arr->_arr + index * sizeof(void*),
    arr->_arr + index * sizeof(void*) + 1,
    move_items * sizeof(void*)
  );
  arr->len = arr->len - 1;

  return old_value_ptr;
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

void DEBUG_dynarray_dump(dynarray_t* arr)
{
  printf("\ndynarray stats:\n");
  printf("address = %p\n", (void*) arr);
  printf("len = %d, capacity = %d, _arr (addr) = %p\n", (int) arr->len, (int) arr->_capacity, (void*) arr->_arr);
  for (size_t i = 0; i < arr->len; i++) {
    printf("arr[%zu] = %p\n", i, dynarray_get(arr, i));
  }
  printf("\n");
}
