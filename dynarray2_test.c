#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <alloca.h>
#include <stdio.h>
#include "dynarray2.h"

static int test_no = 0;

static inline void test(char* name, size_t item_size, void (*fn)(dynarray2_t*))
{
  dynarray2_t* arr = dynarray2_create(item_size);

  printf("%d: %s... ", ++test_no, name);
  fn(arr);
  printf("pass\n");

  dynarray2_free(arr);
}

static void set_get_delete_len(dynarray2_t* arr)
{
  int a = 1, b = 1023, c = -509487738, d = 2763;
  dynarray2_set(arr, 0, &a);
  dynarray2_set(arr, 1, &b);
  dynarray2_set(arr, 2, &c);
  dynarray2_set(arr, 3, &d);

  assert(*dynarray2_get_val(arr, 0).ip == a);
  assert(*dynarray2_get_val(arr, 1).ip == b);
  assert(*dynarray2_get_val(arr, 2).ip == c);
  assert(*dynarray2_get_val(arr, 3).ip == d);
  assert(arr->len == 4);

  dynarray2_delete(arr, 1);
  dynarray2_delete(arr, 1);
  assert(*(int*) dynarray2_get(arr, 0) == a);
  assert(*(int*) dynarray2_get(arr, 1) == d);
  assert(arr->len == 2);
}

static void pointer_correctness(dynarray2_t* arr)
{
  char a = 'a', b = 'b', c = 'c';

  assert(dynarray2_get(arr, 0) == NULL);
  assert(dynarray2_get(arr, 129839030929) == NULL);
  assert(dynarray2_get_top(arr) == NULL);

  dynarray2_set(arr, 0, &a);
  dynarray2_append(arr, &b);
  dynarray2_append(arr, &c);

  assert(dynarray2_get(arr, 0) != NULL);
  assert(dynarray2_get(arr, 1) != NULL);
  assert(dynarray2_get(arr, 2) != NULL);
  assert(dynarray2_get(arr, 2) == dynarray2_get_top(arr));
}

static void multiple_allocations(dynarray2_t* arr)
{
  const int size = DYNARRAY2_ALLOC_STEP * 5;
  int vals[size];
  for (int i = 0; i < size; i++) {
    vals[i] = i * 3;
    dynarray2_set(arr, i, &vals[i]);
  }
  for (int i = 0; i < size; i++) {
    assert(*(int*) dynarray2_get(arr, i) == vals[i]);
  }
  for (int i = 0; i < size; i++) {
    dynarray2_remove_top(arr);
  }
  assert(arr->len == 0);
}

static void top_convenience(dynarray2_t* arr)
{
  long x = 12094837803, y = -19237382803, z = 9238784618263;
  dynarray2_append(arr, &x);
  dynarray2_append(arr, &y);
  dynarray2_append(arr, &z);
  assert(arr->len == 3);

  assert(*(long*) dynarray2_get_top(arr) == z);
  assert(arr->len == 3);

  dynarray2_remove_top(arr);
  assert(*(long*) dynarray2_get_top(arr) == y);
  assert(arr->len == 2);

  assert(*(long*) dynarray2_get_top(arr) == *(long*) dynarray2_get(arr, 1));
}

struct TestStruct {
  int x;
  float y;
  char* str;
  char buf[20];
};

static void arbitrary_len_struct(dynarray2_t* arr)
{
  struct TestStruct a = {.x = 257, .y = 43.12, .str = "A long string", .buf = "Hello"};
  struct TestStruct b = {.x = -100000, .y = 0.00002, .str = "Another str", .buf = "World"};

  dynarray2_append(arr, &a);
  dynarray2_append(arr, &b);

  struct TestStruct* ret_a = dynarray2_get(arr, 0);
  struct TestStruct* ret_b = dynarray2_get(arr, 1);

  assert(ret_a->x == a.x);
  assert(ret_a->y == a.y);
  assert(strcmp(ret_a->str, a.str) == 0);
  assert(strcmp(ret_a->buf, a.buf) == 0);

  assert(ret_b->x == b.x);
  assert(ret_b->y == b.y);
  assert(strcmp(ret_b->str, b.str) == 0);
  assert(strcmp(ret_b->buf, b.buf) == 0);

  dynarray2_delete(arr, 0);
  assert(arr->len == 1);

  struct TestStruct* ret_only = dynarray2_get(arr, 0);
  assert(ret_only->x == b.x);
  assert(ret_only->y == b.y);
  assert(strcmp(ret_only->str, b.str) == 0);
  assert(strcmp(ret_only->buf, b.buf) == 0);
}

static const int ARR_LEN = 733;

static void arbitrary_len_array(dynarray2_t* arr)
{
  float x[ARR_LEN], y[ARR_LEN];
  for (int i = 0; i < ARR_LEN; i++) {
    x[i] = (float) rand() / i;
    y[i] = (float) rand() / rand() * x[i];
  }
  dynarray2_append(arr, &x);
  dynarray2_append(arr, &y);
  for (int i = 0; i < ARR_LEN; i++) {
    assert(x[i] == *(dynarray2_get_val(arr, 0).fp + i));
    assert(y[i] == *(dynarray2_get_val(arr, 1).fp + i));
  }

  dynarray2_remove_top(arr);
  for (int i = 0; i < ARR_LEN; i++) {
    assert(x[i] == *(dynarray2_get_val(arr, 0).fp + i));
  }
}

int main(void)
{
  test("set, get, delete and len work", sizeof(int), set_get_delete_len);
  test("pointers are correct", sizeof(char), pointer_correctness);
  test("multiple allocations work", sizeof(int), multiple_allocations);
  test("convenience methods append, get_top, remove_top work", sizeof(long), top_convenience);
  test("structs of arbitrary length are stored correctly", sizeof(struct TestStruct), arbitrary_len_struct);
  test("arrays of arbitrary length are stored correctly", sizeof(float[ARR_LEN]), arbitrary_len_array);
  return 0;
}
