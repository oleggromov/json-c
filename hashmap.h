#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
  char* key;
  void* value;
} hashmap_value_t;

typedef struct {
  long size;
  long* bucket_sizes;
  hashmap_value_t** bucket_values;
} hashmap_t;

hashmap_t* hashmap_create();
void* hashmap_free(hashmap_t* obj);

void* hashmap_get(hashmap_t* obj, char* key);
void* hashmap_set(hashmap_t* obj, char* key, void* value_ptr);
void* hashmap_del(hashmap_t* obj, char* key);

unsigned int hashmap_count_keys(hashmap_t* obj);
char** hashmap_get_keys(hashmap_t* obj);
double hashmap_get_load_factor(hashmap_t* obj);

#endif
