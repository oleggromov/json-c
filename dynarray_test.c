#include <stdio.h>

#include "dynarray.h"

static void print_stat(dynarray_t* arr)
{
  printf("len = %zu, capacity = %zu\n", arr->len, arr->_capacity);
}

static void print_items_int(dynarray_t* arr)
{
  for (size_t i = 0; i < arr->len; i++) {
    int* value = dynarray_get(arr, i);
    if (value) {
      printf("[%d] = %d\n", (int) i, *value);
    } else {
      printf("[%d] = NULL\n", (int) i);
    }
  }
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

  printf("\n\ndynarray 2\n");
  dynarray_t* arr2 = dynarray_create();
  for (int j = 0; j < 10; j += 2) {
    dynarray_set(arr2, j, &j);
  }
  print_stat(arr2);
  print_items_int(arr2);
  printf("[9] = %d\n", dynarray_get(arr2, 9));
  printf("[10] = %d\n", dynarray_get(arr2, 10));

  int* item = dynarray_cut(arr2, 0);
  printf("cut item = %d\n", *item);
  print_stat(arr2);
  print_items_int(arr2);
  printf("[8] = %d\n", dynarray_get(arr2, 8));
  printf("[9] = %d\n", dynarray_get(arr2, 9));
  printf("[10] = %d\n", dynarray_get(arr2, 10));

  printf("\n\ndynarray 3\n");
  dynarray_t* arr3 = dynarray_create();
  for (int j = 0; j < 10; j++) {
    dynarray_set(arr3, j, &j);
  }
  print_stat(arr3);
  print_items_int(arr3);

  printf("&array[67] = %p\n", dynarray_get(array, 128));

}
