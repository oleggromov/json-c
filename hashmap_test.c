#include <alloca.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"
#include "hashmap.h"

void set_get_del(void* void_obj)
{
  hashmap_t* obj = (hashmap_t*) void_obj;

  char* str = alloca(sizeof("hello world") + 1);
  *str = '\0';
  strcat(str, "hello world");
  long* number = alloca(sizeof(long));
  *number = 218448782393093;
  double* pi = alloca(sizeof(double));
  *pi = 3.14159;

  hashmap_set(obj, "a string", str);
  hashmap_set(obj, "long number", number);
  hashmap_set(obj, "pi", pi);

  assert(hashmap_get(obj, "pi") == pi);
  assert(*(double*) hashmap_get(obj, "pi") == 3.14159);
  assert(hashmap_get(obj, "a string") == str);
  assert(strcmp((char*) hashmap_get(obj, "a string"), "hello world") == 0);
  assert(hashmap_get(obj, "long number") == number);
  assert(*(long*) hashmap_get(obj, "long number") == 218448782393093);

  assert(hashmap_get(obj, "non-key") == NULL);

  hashmap_del(obj, "a string");
  assert(hashmap_get(obj, "a string") == NULL);

  hashmap_del(obj, "pi");
  assert(hashmap_get(obj, "pi") == NULL);
  assert(hashmap_get(obj, "long number") == number);
  hashmap_del(obj, "long number");
  assert(hashmap_get(obj, "long number") == NULL);
}

void keys_over_buckets(void* void_obj)
{
  hashmap_t* obj = (hashmap_t*) void_obj;
  const int size = HASHMAP_SIZE_BUCKETS * 10;
  long *values = alloca(sizeof(long) * size);
  char *keys = alloca(sizeof(char) * 30 * size);

  for (int i = 0; i < size; i++) {
    values[i] = (long) i * 1024;
    sprintf(keys + i * 30, "key-%d", i);
    hashmap_set(obj, keys + i * 30, &values[i]);
  }

  char cur_key[30];
  for (int i = 0; i < size; i++) {
    sprintf(cur_key, "key-%d", i);
    assert(hashmap_get(obj, cur_key) == &values[i]);
    assert(*(long*) hashmap_get(obj, cur_key) == values[i]);
  }
}

void key_methods(void* void_obj)
{
  hashmap_t* obj = (hashmap_t*) void_obj;
  char* str = alloca(sizeof("hello world") + 1);
  *str = '\0';
  strcat(str, "hello world");
  long* number = alloca(sizeof(long));
  *number = 218448782393093;
  double* pi = alloca(sizeof(double));
  *pi = 3.14159;

  hashmap_set(obj, "a string", str);
  hashmap_set(obj, "long number", number);
  hashmap_set(obj, "pi", pi);
  assert(hashmap_count_keys(obj) == 3);

  char* exp_keys[] = {"a string", "long number", "pi"};
  char** ret_keys = hashmap_get_keys(obj);
  test_sort_strings(ret_keys, hashmap_count_keys(obj));
  for (size_t i = 0; i < hashmap_count_keys(obj); i++) {
    assert(strcmp(*(ret_keys + i), *(exp_keys + i)) == 0);
  }

  hashmap_del(obj, "a string");
  assert(hashmap_count_keys(obj) == 2);

  hashmap_del(obj, "non-key");
  hashmap_del(obj, "pi");
  hashmap_del(obj, "pi");
  assert(hashmap_count_keys(obj) == 1);
  ret_keys = hashmap_get_keys(obj);
  assert(strcmp(*ret_keys, "long number") == 0);

  hashmap_del(obj, "long number");
  assert(hashmap_count_keys(obj) == 0);

  ret_keys = hashmap_get_keys(obj);
  assert(ret_keys == NULL);
}

void pointers(void* void_obj)
{
  hashmap_t* obj = (hashmap_t*) void_obj;
  long val1 = 29830193;
  long val2 = -394810;

  assert(hashmap_set(obj, "long value 1", &val1) == NULL);
  assert(hashmap_set(obj, "long value 1", &val2) == &val1);
  assert(hashmap_del(obj, "long value 1") == &val2);
  assert(hashmap_del(obj, "non-key") == NULL);
}

void* b(void)
{
  return (void*) hashmap_create();
}

void a(void* obj)
{
  hashmap_free((hashmap_t**) &obj);
}

int main(void)
{
  test("set, get, del work", set_get_del, b, a);
  test("collision resolution (num keys > buckets) works", keys_over_buckets, b, a);
  test("key count and retrieval works", key_methods, b, a);
  test("set, get, del returned pointers are correct", pointers, b, a);
}
