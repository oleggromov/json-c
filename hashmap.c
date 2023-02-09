#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "hashmap.h"
#include "murmur3_32.h"

hashmap_t* hashmap_create()
{
  const int BUCKETS = 1024;
  hashmap_t* obj = malloc(sizeof(hashmap_t));

  obj->value_ptrs = malloc(BUCKETS * sizeof(hashmap_value_t*));
  obj->bucket_sizes = malloc(BUCKETS * sizeof(long));
  for (long i = 0; i < BUCKETS; i++) {
    obj->bucket_sizes[i] = 0;
  }
  obj->size = BUCKETS;

  return obj;
}

// Always returns NULL and should be used to replace the old pointer
void* hashmap_free(hashmap_t* obj)
{
  for (int bucket_no = 0; bucket_no < obj->size; bucket_no++) {
    for (int i = 0; i < obj->bucket_sizes[bucket_no]; i++) {
      free(obj->value_ptrs[bucket_no][i]); // freeing the hashmap_value_t's
    }
    free(obj->value_ptrs[bucket_no]); // freeing the value array
  }
  free(obj->value_ptrs); // freeing the buckets array
  free(obj->bucket_sizes);
  free(obj); // freeing the struct pointer

  return NULL;
}

static uint32_t hash_key(const char* key)
{
  return murmur3_32((uint8_t*) key, strlen(key), 0x2300FEED);
}

static hashmap_value_t* get_value(const hashmap_t* obj, const long bucket_no, const char* key)
{
  for (long i = 0; i < obj->bucket_sizes[bucket_no]; i++) {
    if (0 == strcmp(key, obj->value_ptrs[bucket_no][i]->key)) {
      return obj->value_ptrs[bucket_no][i];
    }
  }

  return NULL;
}

// Returns
// void* to a value if the key exists
// NULL otherwise
void* hashmap_get(hashmap_t* obj, char* key)
{
  uint32_t key_hash = hash_key(key);
  long bucket_no = key_hash % obj->size;
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
  uint32_t key_hash = hash_key(key);
  long bucket_no = key_hash % obj->size;
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder) {
    void* prev_value_ptr = value_holder->value;
    value_holder->value = value_ptr;
    return prev_value_ptr;
  }

  obj->value_ptrs[bucket_no] = realloc(obj->value_ptrs[bucket_no], (obj->bucket_sizes[bucket_no] + 1) * sizeof(hashmap_value_t*));

  hashmap_value_t* new_value = malloc(sizeof(hashmap_value_t));

  // TODO use strdup
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
void* hashmap_del(hashmap_t* obj, char* key)
{
  uint32_t key_hash = hash_key(key);
  long bucket_no = key_hash % obj->size;
  hashmap_value_t* value_holder = get_value(obj, bucket_no, key);

  if (value_holder == NULL) {
    return NULL;
  }

  void* removed_value = value_holder->value;

  // TODO remove the value + resize the array

  return removed_value;
}

unsigned int hashmap_count_keys(hashmap_t* obj)
{
  unsigned int count = 0;

  for (int bucket_no = 0; bucket_no < obj->size; bucket_no++) {
    count += obj->bucket_sizes[bucket_no];
  }

  return count;
}

char** hashmap_get_keys(hashmap_t* obj)
{
  char** ptr_list = malloc(hashmap_count_keys(obj) * sizeof(char*));

  int key_i = 0;
  for (long bucket_no = 0; bucket_no < obj->size; bucket_no++) {
    for (int i = 0; i < obj->bucket_sizes[bucket_no]; i++) {
      unsigned int key_len = strlen(obj->value_ptrs[bucket_no][i]->key);
      ptr_list[key_i] = malloc(key_len + 1);
      *ptr_list[key_i] = '\0';
      strncat(ptr_list[key_i], obj->value_ptrs[bucket_no][i]->key, key_len);
      key_i++;
    }
  }

  return ptr_list;
}

double hashmap_get_load_factor(hashmap_t* obj)
{
  int used_buckets = 0;
  for (long i = 0; i < obj->size; i++) {
    used_buckets += obj->bucket_sizes[i] > 0 ? 1 : 0;
  }
  return used_buckets / (double)obj->size;
}


static void print_hash(char* str)
{
  unsigned int len = strlen(str);
  unsigned int hash = murmur3_32((uint8_t*) str, len, 0xDEADBEEF);
  printf("0x%08X\t\"%s\"\n", hash, str);
}


int main()
{
  hashmap_t* hashmap = hashmap_create();
  int* value1 = malloc(sizeof(int));
  *value1 = 1024;

  char* str = malloc((strlen("hello world") + 1) * sizeof(char));
  *str = '\0';
  strcat(str, "hello world");

  hashmap_set(hashmap, "test key", value1);
  printf("test key: pointer = %p, value = %d\n", hashmap_get(hashmap, "test key"), *(int*) hashmap_get(hashmap, "test key"));

  hashmap_set(hashmap, "another key", str);
  printf("another key: pointer = %p, value = %s\n", hashmap_get(hashmap, "another key"), hashmap_get(hashmap, "another key"));

  *str = '\0';
  strcat(str, "no");

  printf("another key (after messing with pointer): pointer = %p, value = %s\n", hashmap_get(hashmap, "another key"), hashmap_get(hashmap, "another key"));

  void *replaced = hashmap_set(hashmap, "another key", value1);
  printf("another key (old value): pointer = %p, value = %s\n", replaced, replaced);
  printf("another key (replaced): pointer = %p, value = %d\n", hashmap_get(hashmap, "another key"), *(int*) hashmap_get(hashmap, "another key"));

  printf("load factor = %f\n", hashmap_get_load_factor(hashmap));

  srand(time(NULL));
  for (int i = 0; i < 10000; i++) {
    char* key = malloc(20 * sizeof(char));
    sprintf(key, "key-%d", i);
    int* value = malloc(sizeof(int));
    *value = rand();

    if (i % 1000 == 0) {
      printf("key count = %d, load factor = %f\n", hashmap_count_keys(hashmap), hashmap_get_load_factor(hashmap));    }

    hashmap_set(hashmap, key, value);
  }

  unsigned long key_count = hashmap_count_keys(hashmap);

  printf("key count = %d\n", key_count);
  printf("load factor = %f\n", hashmap_get_load_factor(hashmap));

  // char** keys = hashmap_get_keys(hashmap);

  // for (long i = 0; i < key_count; i++) {
  //   printf("%s ", keys[i]);
  // }
  // printf("\n");


  hashmap = hashmap_free(hashmap);

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
