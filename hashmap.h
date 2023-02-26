#ifndef HASHMAP_H
#define HASHMAP_H

static const size_t HASHMAP_SIZE_BUCKETS = 16;

typedef struct {
  char* key;
  void* value;
} hashmap_value_t;

typedef struct {
  size_t _size;
  size_t* _sizes;
  hashmap_value_t** _values;
} hashmap_t;

hashmap_t* hashmap_create(void);
void hashmap_free(hashmap_t** obj);

void* hashmap_set(hashmap_t* obj, char* key, void* value_ptr);
void* hashmap_get(hashmap_t* obj, char* key);
void* hashmap_del(hashmap_t* obj, char* key);

size_t hashmap_count_keys(hashmap_t* obj);
char** hashmap_get_keys(hashmap_t* obj);
double hashmap_get_load_factor(hashmap_t* obj);

#endif
