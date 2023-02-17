#include "dynarray2.h"

static const size_t ALLOC_STEP = 64;

dynarray2_t* dynarray2_create(size_t item_size)
{
  dynarray2_t* arr = malloc(sizeof(dynarray2_t));
  arr->_mem = NULL;
  arr->_capacity = 0;
  arr->len = 0;
  arr->_item_size = item_size;
  return arr;
}

void dynarray2_free(dynarray2_t* arr)
{
  free(arr->_mem);
  free(arr);
}

// TODO same as _get but without index checks
static inline void* _mem_offset(dynarray2_t* arr, size_t offset) {
  return (uint8_t*) arr->_mem + offset * arr->_item_size;
}

static inline bool _index_within_boundaries(dynarray2_t* arr, ssize_t index)
{
  return index >= 0 && (size_t) index < arr->len;
}

static void _grow_if_needed(dynarray2_t* arr, size_t index)
{
  if (index > arr->_capacity - 1 || arr->_capacity == 0) {
    size_t old_capacity = arr->_capacity;
    size_t size_inc = (index / ALLOC_STEP + 1) * ALLOC_STEP - arr->_capacity;

    arr->_capacity += size_inc;
    arr->_mem = realloc(arr->_mem, arr->_item_size * arr->_capacity);
    if (arr->_mem == NULL) {
      die("dynarray2: couldn't reallocate memory");
    }

    memset(_mem_offset(arr, old_capacity), 0, arr->_item_size * size_inc);
  }
}

// Returns
// - item pointer if index is within [0, len] (may be NULL for sparse arrays)
// - NULL if index is out of bounds
void* dynarray2_get(dynarray2_t* arr, ssize_t index)
{
  if (_index_within_boundaries(arr, index)) {
    return _mem_offset(arr, index);
  }
  return NULL;
}

void dynarray2_set(dynarray2_t* arr, ssize_t index, void* value)
{
  if (index >= 0) {
    _grow_if_needed(arr, index);
    memcpy(_mem_offset(arr, index), value, arr->_item_size);

    // len = 0, index = 0 -> (index == len) + 1
    // len = 3, index = 4 -> index + 1
    // len = 5, index = 3 -> len
    if ((size_t) index >= arr->len) {
      arr->len = index + 1;
    }
  }
}

void dynarray2_delete(dynarray2_t* arr, ssize_t index)
{
  if (_index_within_boundaries(arr, index)) {
    size_t move_items = arr->len - index;
    memmove(
      _mem_offset(arr, index),
      _mem_offset(arr, index + 1),
      move_items * arr->_item_size
    );
    arr->len -= 1;
  }
}

void dynarray2_append(dynarray2_t* arr, void* value)
{
  dynarray2_set(arr, arr->len, value);
}

void* dynarray2_get_top(dynarray2_t* arr)
{
  return dynarray2_get(arr, arr->len - 1);
}

void dynarray2_remove_top(dynarray2_t* arr)
{
  dynarray2_delete(arr, arr->len - 1);
}

void DEBUG_dynarray2_dump(dynarray2_t* arr)
{
  printf("\ndynarray2 stats:\n");
  printf("address = %p\n", (void*) arr);
  printf("item size = %ld\n", arr->_item_size);
  printf("len = %d, capacity = %d, _arr (addr) = %p\n", (int) arr->len, (int) arr->_capacity, (void*) arr->_mem);
  for (size_t i = 0; i < arr->len; i++) {
    printf("arr[%zu] = %p\n", i, dynarray2_get(arr, i));
  }
  printf("\n");
}
