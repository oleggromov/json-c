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

void* get(hashmap_t* obj, char* key);
void* set(hashmap_t* obj, char* key, void* value_ptr);
void* del(hashmap_t* obj, char* key);

hashmap_t* create_hashmap();
void free_hashmap(hashmap_t* obj);

#endif
