#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "murmur3_32.h"
#include "util.h"

typedef unsigned long hashmap_size_t;

typedef struct {
  char* key;
  void* value;
} hashmap_value_t;

typedef struct {
  hashmap_size_t size;
  hashmap_size_t* sizes;
  hashmap_value_t** values;
} hashmap_t;

hashmap_t* hashmap_create(void);
void hashmap_free(hashmap_t** obj);

void* hashmap_get(hashmap_t* obj, char* key);
void* hashmap_set(hashmap_t* obj, char* key, void* value_ptr);
void* hashmap_del(hashmap_t* obj, char* key);

hashmap_size_t hashmap_count_keys(hashmap_t* obj);
char** hashmap_get_keys(hashmap_t* obj);
double hashmap_get_load_factor(hashmap_t* obj);

#endif
