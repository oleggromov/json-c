#include <sys/types.h>

#ifndef DYNARRAY2_H
#define DYNARRAY2_H

static const size_t DYNARRAY2_ALLOC_STEP = 16;

typedef struct {
  size_t len;
  size_t _capacity;
  size_t _item_size;
  void* _mem;
} dynarray2_t;

typedef union {
  void* p;
  char* cp;
  unsigned char* ucp;
  int* ip;
  unsigned int* uip;
  long* lp;
  unsigned long* ulp;
  float* fp;
  double* dp;
  long double* ldp;
} dynarray2_ptr_t;

dynarray2_t* dynarray2_create(size_t item_size);
void dynarray2_free(dynarray2_t* arr);

void* dynarray2_get(dynarray2_t* arr, ssize_t index);
void* dynarray2_get_top(dynarray2_t* arr);

dynarray2_ptr_t dynarray2_get_val(dynarray2_t* arr, ssize_t index);
dynarray2_ptr_t dynarray2_get_top_val(dynarray2_t* arr);

void dynarray2_set(dynarray2_t* arr, ssize_t index, void* value);
void dynarray2_delete(dynarray2_t* arr, ssize_t index);
void dynarray2_append(dynarray2_t* arr, void* value);
void dynarray2_remove_top(dynarray2_t* arr);

void DEBUG_dynarray2_dump(dynarray2_t* arr);

#endif
