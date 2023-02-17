#include <alloca.h> // for testing only!

#include "hashmap.h"

hashmap_t* hashmap_create(void)
{
  const hashmap_size_t BUCKETS = 1024;
  hashmap_t* obj = malloc(sizeof(hashmap_t));

  obj->values = calloc(BUCKETS, sizeof(hashmap_value_t*)); // 0-ing pointers
  obj->sizes = calloc(BUCKETS, sizeof(long)); // 0-ing sizes

  if (obj->values == NULL || obj->sizes == NULL) {
    die("hashmap_create: allocation failed");
  }

  obj->size = BUCKETS;

  return obj;
}

// Replaces the obj pointer with NULL
void hashmap_free(hashmap_t** obj_ptr)
{
  for (hashmap_size_t bucket_no = 0; bucket_no < (*obj_ptr)->size; bucket_no++) {
    free((*obj_ptr)->values[bucket_no]); // freeing the hashmap_value_t array
  }
  free((*obj_ptr)->values); // freeing the buckets array
  free((*obj_ptr)->sizes);
  free(*obj_ptr); // freeing the struct pointer

  obj_ptr = NULL;
}

static inline hashmap_size_t get_bucket_no(const hashmap_t* obj, const char* key)
{
  uint32_t hash = murmur3_32((uint8_t*) key, strlen(key), 0x2300FEED);
  return hash % obj->size;
}

static hashmap_value_t* get_value(const hashmap_t* obj, const hashmap_size_t bucket_no, const char* key)
{
  for (hashmap_size_t i = 0; i < obj->sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->values[bucket_no][i].key)) {
      return &obj->values[bucket_no][i];
    }
  }

  return NULL;
}

// Returns
// void* to a value if the key exists
// NULL otherwise
void* hashmap_get(hashmap_t* obj, char* key)
{
  hashmap_size_t bucket_no = get_bucket_no(obj, key);
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
  hashmap_size_t bucket_no = get_bucket_no(obj, key);
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    void* prev_value_ptr = value_holder->value;
    value_holder->value = value_ptr;
    return prev_value_ptr;
  }

  // Upsize the value array
  obj->values[bucket_no] = realloc(obj->values[bucket_no], (obj->sizes[bucket_no] + 1) * sizeof(hashmap_value_t));

  hashmap_value_t* new_value_ptr = &obj->values[bucket_no][obj->sizes[bucket_no]];

  // TODO use strdup
  new_value_ptr->key = malloc((strlen(key) + 1) * sizeof(char));
  *new_value_ptr->key = '\0';
  strcat(new_value_ptr->key, key);
  new_value_ptr->value = value_ptr;

  obj->sizes[bucket_no] += 1;

  return NULL;
}

// Returns
// - void* to a value was removed
// - NULL if element wasn't found
void* hashmap_del(hashmap_t* obj, char* key)
{
  hashmap_size_t bucket_no = get_bucket_no(obj, key);
  int removed_i = -1;
  void* removed_value_ptr = NULL;

  // Find the value in a bucket, deallocate its key and store the pointer
  for (hashmap_size_t i = 0; i < obj->sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->values[bucket_no][i].key)) {
      free(obj->values[bucket_no][i].key);
      removed_value_ptr = obj->values[bucket_no][i].value;
      removed_i = i;
    }
  }

  if (removed_i > -1) {
    long subsequent_count = obj->sizes[bucket_no] - removed_i - 1;

    // Move subsequent values to i-1 each
    if (subsequent_count > 0) {
      memmove(&obj->values[bucket_no][removed_i], &obj->values[bucket_no][removed_i + 1], subsequent_count * sizeof(hashmap_value_t));
    }

    // Downsize the array
    obj->sizes[bucket_no] -= 1;
    obj->values[bucket_no] = realloc(obj->values[bucket_no], obj->sizes[bucket_no] * sizeof(hashmap_value_t));

    return removed_value_ptr;
  }

  return NULL;
}

hashmap_size_t hashmap_count_keys(hashmap_t* obj)
{
  hashmap_size_t count = 0;

  for (hashmap_size_t bucket_no = 0; bucket_no < obj->size; bucket_no++) {
    count += obj->sizes[bucket_no];
  }

  return count;
}

char** hashmap_get_keys(hashmap_t* obj)
{
  char** ptr_list = malloc(hashmap_count_keys(obj) * sizeof(char*));

  int key_i = 0;
  for (hashmap_size_t bucket_no = 0; bucket_no < obj->size; bucket_no++) {
    for (hashmap_size_t i = 0; i < obj->sizes[bucket_no]; i++) {
      unsigned int key_len = strlen(obj->values[bucket_no][i].key);
      ptr_list[key_i] = malloc(key_len + 1);
      *ptr_list[key_i] = '\0';
      strncat(ptr_list[key_i], obj->values[bucket_no][i].key, key_len);
      key_i++;
    }
  }

  return ptr_list;
}

double hashmap_get_load_factor(hashmap_t* obj)
{
  hashmap_size_t used_buckets = 0;
  for (hashmap_size_t i = 0; i < obj->size; i++) {
    used_buckets += obj->sizes[i] > 0 ? 1 : 0;
  }
  return (double)used_buckets / obj->size;
}
