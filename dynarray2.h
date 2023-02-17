#ifndef DYNARRAY2_H
#define DYNARRAY2_H

#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "util.h"

typedef struct {
  size_t len;
  size_t _capacity;
  size_t _item_size;
  void* _mem;
} dynarray2_t;

dynarray2_t* dynarray2_create(size_t item_size);
void dynarray2_free(dynarray2_t* arr);

void* dynarray2_get(dynarray2_t* arr, ssize_t index);
void* dynarray2_get_top(dynarray2_t* arr);

void dynarray2_set(dynarray2_t* arr, ssize_t index, void* value);
void dynarray2_delete(dynarray2_t* arr, ssize_t index);
void dynarray2_append(dynarray2_t* arr, void* value);
void dynarray2_remove_top(dynarray2_t* arr);

void DEBUG_dynarray2_dump(dynarray2_t* arr);

#endif
