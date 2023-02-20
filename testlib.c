#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "testlib.h"

static int test_no = 0;

void test(char* name, void (*test_case)(void*), void* (*before)(void), void (*after)(void*))
{
  void* obj = before();

  printf("%d: %s... ", ++test_no, name);
  test_case(obj);
  printf("pass\n");

  after(obj);
}

void expect(bool cond, char* msg)
{

}

static inline int str_comp(const void* s1, const void* s2)
{
  return strcmp(*(char**) s1, *(char**) s2);
}

void test_sort_strings(char** arr, size_t size)
{
  qsort(arr, size, sizeof(char*), str_comp);
}
