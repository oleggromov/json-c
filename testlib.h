#ifndef JSON_TESTLIB_H
#define JSON_TESTLIB_H

#include <assert.h>
#include <stdbool.h>

void test(char* name, void (*test_case)(void*), void* (*before)(void), void (*after)(void*));

void test_sort_strings(char** arr, size_t size);

#endif
