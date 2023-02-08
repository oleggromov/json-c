#include <string.h>
#include <stdlib.h>

#include "hashmap.h"
#include "murmur3_32.h"

hashmap_t* create_hashmap()
{
  const int BUCKETS = 1024;
  hashmap_t* obj = malloc(sizeof(hashmap_t));

  obj->value_ptrs = malloc(BUCKETS * sizeof(hashmap_value_t*));
  obj->bucket_sizes = malloc(BUCKETS * sizeof(hashmap_size_t));
  for (hashmap_size_t i = 0; i < BUCKETS; i++) {
    obj->bucket_sizes[i] = 0;
  }
  obj->size = BUCKETS;

  return obj;
}

void free_hashmap(hashmap_t* obj)
{
  // TODO
}

static uint32_t hash_key(const char* key)
{
  return murmur3_32((uint8_t*) key, strlen(key), 0x2300FEED);
}

static hashmap_value_t* get_value(const hashmap_t* obj, const hashmap_size_t bucket_no, const char* key)
{
  for (hashmap_size_t i = 0; i < obj->bucket_sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->value_ptrs[bucket_no][i]->key)) {
      return obj->value_ptrs[bucket_no][i];
    }
  }

  return NULL;
}

void* get(hashmap_t* obj, char* key)
{
  uint32_t key_hash = hash_key(key);
  hashmap_size_t bucket_no = key_hash % obj->size;
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    return value_holder->value;
  }

  return NULL;
}

// Returns
// - NULL if value is appended
// - void* to a previous value if replaced
void* set(hashmap_t* obj, char* key, void* value_ptr)
{
  uint32_t key_hash = hash_key(key);
  hashmap_size_t bucket_no = key_hash % obj->size;
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    void* prev_value_ptr = value_holder->value;
    value_holder->value = value_ptr;
    return prev_value_ptr;
  }

  obj->value_ptrs[bucket_no] = realloc(obj->value_ptrs[bucket_no], (obj->bucket_sizes[bucket_no] + 1) * sizeof(hashmap_value_t*));

  hashmap_value_t* new_value = malloc(sizeof(hashmap_value_t));

  new_value->key = malloc((strlen(key) + 1) * sizeof(char));
  *new_value->key = '\0';
  strcat(new_value->key, key);
  new_value->value = value_ptr;

  obj->value_ptrs[bucket_no][obj->bucket_sizes[bucket_no]] = new_value;
  obj->bucket_sizes[bucket_no] += 1;

  return NULL;
}

// Returns
// - void* to a value was removed
// - NULL if element wasn't found
void* del(hashmap_t* obj, char* key)
{
  uint32_t key_hash = hash_key(key);
  hashmap_size_t bucket_no = key_hash % obj->size;
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder == NULL) {
    return NULL;
  }

  void* removed_value = value_holder->value;

  // TODO remove the value + resize the array

  return removed_value;
}


static void print_hash(char* str)
{
  unsigned int len = strlen(str);
  unsigned int hash = murmur3_32((uint8_t*) str, len, 0xDEADBEEF);
  printf("0x%08X\t\"%s\"\n", hash, str);
}


int main()
{
  hashmap_t* hashmap = create_hashmap();
  int* value1 = malloc(sizeof(int));
  *value1 = 1024;

  char* str = malloc((strlen("hello world") + 1) * sizeof(char));
  *str = '\0';
  strcat(str, "hello world");


  set(hashmap, "test key", value1);
  printf("test key: pointer = %p, value = %d\n", get(hashmap, "test key"), *(int*) get(hashmap, "test key"));

  set(hashmap, "another key", str);
  printf("another key: pointer = %p, value = %s\n", get(hashmap, "another key"), get(hashmap, "another key"));

  *str = '\0';
  strcat(str, "no");

  printf("another key (after messing with pointer): pointer = %p, value = %s\n", get(hashmap, "another key"), get(hashmap, "another key"));

  void *replaced = set(hashmap, "another key", value1);
  printf("another key (old value): pointer = %p, value = %s\n", replaced, replaced);
  printf("another key (replaced): pointer = %p, value = %d\n", get(hashmap, "another key"), *(int*) get(hashmap, "another key"));


  // print_hash("");
  // print_hash("hello, world");
  // print_hash("hello, world!");
  // print_hash("good bye, world!");
  // print_hash("1");
  // print_hash("2");
  // print_hash("12");
  // print_hash("true");

  return 0;
}
