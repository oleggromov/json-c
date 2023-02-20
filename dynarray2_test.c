#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <alloca.h>
#include <stdio.h>

#include "testlib.h"
#include "dynarray2.h"

static void set_get_delete_len(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void pointer_correctness(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void negative_indices(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
  char a = 'X', b = '2', c = '~';
  dynarray2_set(arr, 0, &a);
  dynarray2_append(arr, &b);
  dynarray2_append(arr, &c);

  dynarray2_get(arr, -255);
  dynarray2_set(arr, -3, &c);
  dynarray2_delete(arr, -1);

  assert(*(char*) dynarray2_get(arr, 0) == a);
  assert(*(char*) dynarray2_get(arr, 1) == b);
  assert(*(char*) dynarray2_get(arr, 2) == c);
  assert(arr->len == 3);
}

static void multiple_allocations(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void top_convenience(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void arbitrary_len_struct(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void arbitrary_len_array(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
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

static void sparse_arrays(void* void_obj)
{
  dynarray2_t* arr = (dynarray2_t*) void_obj;
  int a = 5, b = 203, c = -48930, d = 283861;
  dynarray2_set(arr, 0, &a);
  assert(arr->len == 1);

  dynarray2_set(arr, 250, &b);
  assert(arr->len == 251);
  dynarray2_append(arr, &c);
  assert(arr->len == 252);

  dynarray2_set(arr, 590, &d);
  assert(arr->len == 591);

  for (size_t i = 0; i < arr->len; i++) {
    switch (i) {
      case 0:
        assert(*dynarray2_get_val(arr, i).ip == a);
        break;
      case 250:
        assert(*dynarray2_get_val(arr, i).ip == b);
        break;
      case 251:
        assert(*dynarray2_get_val(arr, i).ip == c);
        break;
      case 590:
        assert(*dynarray2_get_val(arr, i).ip == d);
        break;
      default:
        // Should it be 0s really?
        assert(*dynarray2_get_val(arr, i).ip == 0);
    }
  }

  dynarray2_delete(arr, 200);
  dynarray2_delete(arr, 201);
  assert(arr->len == 589);

  assert(*dynarray2_get_val(arr, 249).ip == c);
  dynarray2_delete(arr, 249);
  assert(*dynarray2_get_val(arr, 249).ip == 0);
  assert(arr->len == 588);

  assert(*dynarray2_get_val(arr, 0).ip == a);
  dynarray2_delete(arr, 0);
  assert(*dynarray2_get_val(arr, 0).ip == 0);
  assert(arr->len == 587);

  assert(*dynarray2_get_val(arr, 247).ip == b);
  assert(*dynarray2_get_val(arr, 586).ip == d);
  assert(*dynarray2_get_top_val(arr).ip == d);
}

void* b_int() { return (void*) dynarray2_create(sizeof(int)); }
void* b_char() { return (void*) dynarray2_create(sizeof(char)); }
void* b_long() { return (void*) dynarray2_create(sizeof(long)); }
void* b_struct() { return (void*) dynarray2_create(sizeof(struct TestStruct)); }
void* b_flarr() { return (void*) dynarray2_create(sizeof(float[ARR_LEN])); }

void a(void* obj) { dynarray2_free((dynarray2_t*) obj); }

int main(void)
{
  test("set, get, delete and len work", set_get_delete_len, b_int, a);
  test("pointers are correct", pointer_correctness, b_char, a);
  test("negative indices don't break stuff", negative_indices, b_char, a);
  test("multiple allocations work",multiple_allocations, b_int, a);
  test("convenience methods append, get_top, remove_top work", top_convenience, b_long, a);
  test("structs of arbitrary length are stored correctly", arbitrary_len_struct, b_struct, a);
  test("arrays of arbitrary length are stored correctly", arbitrary_len_array, b_flarr, a);
  test("sparse arrays work", sparse_arrays, b_int, a);
}
