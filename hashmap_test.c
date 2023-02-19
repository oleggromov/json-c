#include <alloca.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h"

static int test_no = 0;

static inline void test(char* name, void (*fn)(hashmap_t*))
{
  hashmap_t* hashmap = hashmap_create();

  printf("%d: %s... ", ++test_no, name);
  fn(hashmap);
  printf("pass\n");

  hashmap_free(&hashmap);
}

void set_get_del(hashmap_t* obj)
{
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

void keys_over_buckets(hashmap_t* obj)
{
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

static inline int str_comp(const void* s1, const void* s2)
{
  return strcmp(*(char**) s1, *(char**) s2);
}

void key_methods(hashmap_t* obj)
{
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
  qsort(ret_keys, hashmap_count_keys(obj), sizeof(char*), str_comp);
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

void pointers(hashmap_t* obj)
{
  long val1 = 29830193;
  long val2 = -394810;

  assert(hashmap_set(obj, "long value 1", &val1) == NULL);
  assert(hashmap_set(obj, "long value 1", &val2) == &val1);
  assert(hashmap_del(obj, "long value 1") == &val2);
  assert(hashmap_del(obj, "non-key") == NULL);
}

int main(void)
{
  test("set, get, del work", set_get_del);
  test("collision resolution (num keys > buckets) works", keys_over_buckets);
  test("key count and retrieval works", key_methods);
  test("set, get, del returned pointers are correct", pointers);
}
