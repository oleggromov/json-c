#include <stdio.h>

#include "dynarray.h"

static void print_stat(dynarray_t* arr)
{
  printf("len = %zu, capacity = %zu\n", arr->len, arr->_capacity);
}

int main()
{
  dynarray_t* array = dynarray_create();
  print_stat(array);

  int x = 4;
  long l = 20483034004;

  dynarray_append(array, &x);
  dynarray_append(array, &l);
  printf("array[0] = %d\n", *(int*) dynarray_get(array, 0));
  printf("array[1] = %ld\n", *(long*) dynarray_get(array, 1));
  print_stat(array);

  int i;
  for (i = 0; i < 64; i++) {
    dynarray_append(array, &i);
  }

  printf("array[64] = %d\n", *(int*) dynarray_get(array, 64));
  printf("array[65] = %d\n", *(int*) dynarray_get(array, 65));
  print_stat(array);

  printf("&array[66] = %p\n", dynarray_get(array, 66));
  printf("&array[67] = %p\n", dynarray_get(array, 67));
  printf("&array[67] = %p\n", dynarray_get(array, 128));
}
