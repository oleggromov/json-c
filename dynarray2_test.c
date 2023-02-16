#include <alloca.h>
#include <stdio.h>
#include "dynarray2.h"

struct X {
  long number;
  char* str;
  double double_number;
};

void print_long_array(dynarray2_t* arr)
{
  printf("Array len = %ld, capacity = %ld\n", arr->len, arr->_capacity);
  printf("(");
  for (size_t i = 0; i < arr->len; i++) {
    printf("%ld ", *(long*) dynarray2_get(arr, i));
  }
  printf(")\n");
}

int main() {
  // int a = 1;
  // int b = 2;
  // int c = 3;

  // dynarray2_t* arr = dynarray2_create(sizeof(int));
  // dynarray2_append(arr, &a);
  // dynarray2_append(arr, &b);
  // dynarray2_append(arr, &c);

  // DEBUG_dynarray2_dump(arr);

  // printf("0 = %d\n", *(int*) dynarray2_get(arr, 0));
  // printf("1 = %d\n", *(int*) dynarray2_get(arr, 1));
  // printf("2 = %d\n", *(int*) dynarray2_get(arr, 2));

  // int d = 4;
  // int e = 5;

  // dynarray2_set(arr, 0, &d);
  // dynarray2_set(arr, 1, &e);

  // printf("0 = %d\n", *(int*) dynarray2_get(arr, 0));
  // printf("1 = %d\n", *(int*) dynarray2_get(arr, 1));
  // printf("2 = %d\n", *(int*) dynarray2_get(arr, 2));

  // int f = 6;

  // dynarray2_append(arr, &f);
  // printf("0 = %d\n", *(int*) dynarray2_get(arr, 0));
  // printf("1 = %d\n", *(int*) dynarray2_get(arr, 1));
  // printf("2 = %d\n", *(int*) dynarray2_get(arr, 2));
  // printf("3 = %d\n", *(int*) dynarray2_get(arr, 3));

  // printf("Structs...\n\n");

  // struct X x = {.number = 15, .str = alloca(sizeof(char)*20), .double_number = 2.55};
  // struct X y = {.number = -1023, .str = alloca(sizeof(char)*20), .double_number = 33.95};

  // dynarray2_t* array = dynarray2_create(sizeof(struct X));

  // dynarray2_set(array, 0, &x);
  // dynarray2_set(array, 1, &y);

  // DEBUG_dynarray2_dump(array);

  // struct X* ret_x = dynarray2_get(array, 0);
  // struct X* ret_y = dynarray2_get(array, 1);

  // printf("Storing structs...\n");
  // printf("x address = %p, ret_x address = %p\n", (void*) &x, (void*) ret_x);
  // printf("x:\n");
  // printf("x.number = %ld\n", x.number);
  // printf("x.str = %p\n", (void*) x.str);
  // printf("x.double_number = %f\n", x.double_number);

  // printf("\nret_x:\n");
  // printf("ret_x.number = %ld\n", ret_x->number);
  // printf("ret_x.str = %p\n", (void*) ret_x->str);
  // printf("ret_x.double_number = %f\n", ret_x->double_number);

  // printf("\nret_y:\n");
  // printf("ret_y.number = %ld\n", ret_y->number);
  // printf("ret_y.str = %p\n", (void*) ret_y->str);
  // printf("ret_y.double_number = %f\n", ret_y->double_number);

  printf("\nLong numbers...\n");
  long num1 = 4;
  long num2 = 1024;

  dynarray2_t* array_longs = dynarray2_create(sizeof(long));

  dynarray2_set(array_longs, 0, &num1);
  dynarray2_set(array_longs, 1, &num2);
  print_long_array(array_longs);

  num2 -= 512;
  dynarray2_set(array_longs, 10, &num2);

  num1 = 665;
  long* existing_num = dynarray2_set(array_longs, 1, &num1);
  printf("removed item [1] = %ld, new item [1] = %ld\n", *existing_num, *(long*) dynarray2_get(array_longs, 1));

  printf("array[top] = %ld\n", *(long*) dynarray2_get_top(array_longs));

  print_long_array(array_longs);

  printf("Appending 10 long items...\n");
  for (long i = 0; i < 10; i++) {
    long oh = (i + 1) * 1000;
    dynarray2_append(array_longs, &oh);
  }

  print_long_array(array_longs);

  printf("array top = %ld, ptr = %p\n", *(long*) dynarray2_get_top(array_longs), dynarray2_get_top(array_longs));

  print_long_array(array_longs);

  printf("\nRemoving two items from the top and popping one...\n");

  printf("%ld\n", *(long*) dynarray2_remove_top(array_longs));
  printf("%ld\n", *(long*) dynarray2_remove_top(array_longs));
  print_long_array(array_longs);

  printf("Removing 5 items from the end...\n");
  for (size_t i = 0; i < 5; i++) {
    printf("i = %ld: %ld\n", i, *(long*) dynarray2_remove_top(array_longs));
  }

  print_long_array(array_longs);
}
