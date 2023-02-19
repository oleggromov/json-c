#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "murmur3_32.h"
#include "util.h"

#include "hashmap.h"

hashmap_t* hashmap_create(void)
{
  hashmap_t* obj = malloc(sizeof(hashmap_t));

  obj->_values = calloc(HASHMAP_SIZE_BUCKETS, sizeof(hashmap_value_t*)); // 0-ing pointers
  obj->_sizes = calloc(HASHMAP_SIZE_BUCKETS, sizeof(long)); // 0-ing sizes

  if (obj->_values == NULL || obj->_sizes == NULL) {
    die("hashmap_create: allocation failed");
  }

  obj->_size = HASHMAP_SIZE_BUCKETS;

  return obj;
}

// Replaces the obj pointer with NULL
void hashmap_free(hashmap_t** obj_ptr)
{
  for (size_t bucket_no = 0; bucket_no < (*obj_ptr)->_size; bucket_no++) {
    free((*obj_ptr)->_values[bucket_no]); // freeing the hashmap_value_t array
  }
  free((*obj_ptr)->_values); // freeing the buckets array
  free((*obj_ptr)->_sizes);
  free(*obj_ptr); // freeing the struct pointer

  obj_ptr = NULL;
}

static inline size_t get_bucket_no(const hashmap_t* obj, const char* key)
{
  uint32_t hash = murmur3_32((uint8_t*) key, strlen(key), 0x2300FEED);
  return hash % obj->_size;
}

static hashmap_value_t* get_value(const hashmap_t* obj, const size_t bucket_no, const char* key)
{
  for (size_t i = 0; i < obj->_sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->_values[bucket_no][i].key)) {
      return &obj->_values[bucket_no][i];
    }
  }

  return NULL;
}

// Returns
// void* to a value if the key exists
// NULL otherwise
void* hashmap_get(hashmap_t* obj, char* key)
{
  size_t bucket_no = get_bucket_no(obj, key);
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    return value_holder->value;
  }

  return NULL;
}

// Returns
// - NULL if value is appended
// - void* to a previous value if replaced
void* hashmap_set(hashmap_t* obj, char* key, void* value_ptr)
{
  size_t bucket_no = get_bucket_no(obj, key);
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    void* prev_value_ptr = value_holder->value;
    value_holder->value = value_ptr;
    return prev_value_ptr;
  }

  // Upsize the value array
  obj->_values[bucket_no] = realloc(obj->_values[bucket_no], (obj->_sizes[bucket_no] + 1) * sizeof(hashmap_value_t));

  hashmap_value_t* new_value_ptr = &obj->_values[bucket_no][obj->_sizes[bucket_no]];

  // TODO use strdup
  new_value_ptr->key = malloc((strlen(key) + 1) * sizeof(char));
  *new_value_ptr->key = '\0';
  strcat(new_value_ptr->key, key);
  new_value_ptr->value = value_ptr;

  obj->_sizes[bucket_no] += 1;

  return NULL;
}

// Returns
// - void* to a value was removed
// - NULL if element wasn't found
void* hashmap_del(hashmap_t* obj, char* key)
{
  size_t bucket_no = get_bucket_no(obj, key);
  int removed_i = -1;
  void* removed_value_ptr = NULL;

  // Find the value in a bucket, deallocate its key and store the pointer
  for (size_t i = 0; i < obj->_sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->_values[bucket_no][i].key)) {
      free(obj->_values[bucket_no][i].key);
      removed_value_ptr = obj->_values[bucket_no][i].value;
      removed_i = i;
    }
  }

  if (removed_i > -1) {
    long subsequent_count = obj->_sizes[bucket_no] - removed_i - 1;

    // Move subsequent values to i-1 each
    if (subsequent_count > 0) {
      memmove(&obj->_values[bucket_no][removed_i], &obj->_values[bucket_no][removed_i + 1], subsequent_count * sizeof(hashmap_value_t));
    }

    // Downsize the array
    obj->_sizes[bucket_no] -= 1;
    obj->_values[bucket_no] = realloc(obj->_values[bucket_no], obj->_sizes[bucket_no] * sizeof(hashmap_value_t));

    return removed_value_ptr;
  }

  return NULL;
}

size_t hashmap_count_keys(hashmap_t* obj)
{
  size_t count = 0;

  for (size_t bucket_no = 0; bucket_no < obj->_size; bucket_no++) {
    count += obj->_sizes[bucket_no];
  }

  return count;
}

char** hashmap_get_keys(hashmap_t* obj)
{
  size_t key_count = hashmap_count_keys(obj);

  if (key_count > 0) {
    char** ptr_list = malloc(key_count * sizeof(char*));

    int key_i = 0;
    for (size_t bucket_no = 0; bucket_no < obj->_size; bucket_no++) {
      for (size_t i = 0; i < obj->_sizes[bucket_no]; i++) {
        unsigned int key_len = strlen(obj->_values[bucket_no][i].key);
        ptr_list[key_i] = malloc(key_len + 1);
        *ptr_list[key_i] = '\0';
        strncat(ptr_list[key_i], obj->_values[bucket_no][i].key, key_len);
        key_i++;
      }
    }

    return ptr_list;
  }

  return NULL;
}

double hashmap_get_load_factor(hashmap_t* obj)
{
  size_t used_buckets = 0;
  for (size_t i = 0; i < obj->_size; i++) {
    used_buckets += obj->_sizes[i] > 0 ? 1 : 0;
  }
  return (double)used_buckets / obj->_size;
}
