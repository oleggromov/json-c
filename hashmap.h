#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdint.h>

typedef unsigned long hashmap_size_t;

typedef struct {
  char* key;
  void* value;
} hashmap_value_t;

typedef struct {
  hashmap_size_t size;
  hashmap_size_t* bucket_sizes;
  hashmap_value_t*** value_ptrs;
} hashmap_t;

void* hashmap_get(hashmap_t* obj, char* key);
void* hashmap_set(hashmap_t* obj, char* key, void* value_ptr);
void* hashmap_del(hashmap_t* obj, char* key);

hashmap_t* hashmap_create();
void hashmap_free(hashmap_t* obj);

#endif
