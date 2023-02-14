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

  printf("Appending 64 items...\n");
  int i;
  for (i = 0; i < 64; i++) {
    dynarray_append(array, &i);
  }

  printf("array[64] = %d\n", *(int*) dynarray_get(array, 64));
  printf("array[65] = %d\n", *(int*) dynarray_get(array, 65));
  printf("&array[66] = %p\n", dynarray_get(array, 66));
  printf("&array[67] = %p\n", dynarray_get(array, 67));
  print_stat(array);

  printf("Popping 3 items...\n");
  for (i = 0; i < 3; i++) {
    printf("array[%zu] = %d\n", array->len - 1, *(int*) dynarray_remove_top(array));
  }
  print_stat(array);

  printf("&array[67] = %p\n", dynarray_get(array, 128));
}
