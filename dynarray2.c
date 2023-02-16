#include "dynarray2.h"

const size_t SIZE_INC_STEP = 64;

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

static void* _get_item_copy(dynarray2_t* arr, size_t index)
{
  void* value_ptr = dynarray2_get(arr, index);

  if (value_ptr != NULL) {
    void* value = malloc(arr->_item_size);
    memcpy(value, value_ptr, arr->_item_size);
    return value;
  }

  return NULL;
}

// TODO same as _get but without index checks
static inline void* _get_item_ptr(dynarray2_t* arr, size_t offset) {
  return (uint8_t*) arr->_mem + offset * arr->_item_size;
}

static bool _assert_index(dynarray2_t* arr, size_t index)
{
  return !(index > arr->_capacity - 1 || index < 0);
}

static void _grow_if_needed(dynarray2_t* arr, size_t index)
{
  if (index > arr->_capacity - 1 || arr->_capacity == 0) {
    size_t old_capacity = arr->_capacity;
    size_t size_inc = (index / SIZE_INC_STEP + 1) * SIZE_INC_STEP - arr->_capacity;

    arr->_capacity += size_inc;
    arr->_mem = realloc(arr->_mem, arr->_item_size * arr->_capacity);
    if (arr->_mem == NULL) {
      die("dynarray2: couldn't reallocate memory");
    }

    memset(_get_item_ptr(arr, old_capacity), 0, arr->_item_size * size_inc);
  }
}

void* dynarray2_get(dynarray2_t* arr, size_t index)
{
  if (_assert_index(arr, index)) {
    return _get_item_ptr(arr, index);
  }
  return NULL;
}

void* dynarray2_set(dynarray2_t* arr, size_t index, void* value)
{
  _grow_if_needed(arr, index);

  void* old_value = _get_item_copy(arr, index);

  memcpy(_get_item_ptr(arr, index), value, arr->_item_size);

  // len = 0, index = 0 -> (index == len) + 1
  // len = 3, index = 4 -> index + 1
  // len = 5, index = 3 -> len
  if (index >= arr->len) {
    arr->len = index + 1;
  }

  return old_value;
}

// Has to be freed!
void* dynarray2_cut(dynarray2_t* arr, size_t index)
{
  void* old_value = _get_item_copy(arr, index);

  if (old_value != NULL) {
    size_t move_items = arr->len - index;
    memmove(
      _get_item_ptr(arr, index),
      _get_item_ptr(arr, index + 1),
      move_items * arr->_item_size
    );
    arr->len -= 1;
  }

  return old_value;
}

void* dynarray2_append(dynarray2_t* arr, void* value)
{
  return dynarray2_set(arr, arr->len, value);
}

void* dynarray2_get_top(dynarray2_t* arr)
{
  return dynarray2_get(arr, arr->len - 1);
}

void* dynarray2_remove_top(dynarray2_t* arr)
{
  return dynarray2_cut(arr, arr->len - 1);
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
