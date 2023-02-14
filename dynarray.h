#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "stddef.h"

typedef struct {
  size_t len;
  size_t _capacity;
  void** _arr;
} dynarray_t;

dynarray_t* dynarray_create(void);
void dynarray_free(dynarray_t** arr);

void* dynarray_set(dynarray_t* arr, size_t index, void* value);
void* dynarray_get(dynarray_t* arr, size_t index);
void* dynarray_cut(dynarray_t* arr, size_t index);

void dynarray_append(dynarray_t* arr, void* value);
void* dynarray_get_top(dynarray_t* arr);
void* dynarray_remove_top(dynarray_t* arr);

#endif
