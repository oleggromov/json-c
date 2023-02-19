#include <stdio.h>
#include <stdlib.h>
#include "hashmap.h"


static void print_hash(char* str)
{
  unsigned int len = strlen(str);
  unsigned int hash = murmur3_32((uint8_t*) str, len, 0xDEADBEEF);
  printf("0x%08X\t\"%s\"\n", hash, str);
}


int main()
{
  hashmap_t* hashmap = hashmap_create();
  int value1 = 1024;

  char* str = alloca((strlen("hello world") + 1) * sizeof(char));
  *str = '\0';
  strcat(str, "hello world");

  printf("[stack] value1=%p\n", &value1);
  printf("[heap] str=%p\n", str);

  hashmap_set(hashmap, "test key", &value1);
  printf("test key: pointer = %p, value = %d\n", hashmap_get(hashmap, "test key"), *(int*) hashmap_get(hashmap, "test key"));

  hashmap_set(hashmap, "another key", str);
  printf("another key: pointer = %p, value = %s\n", hashmap_get(hashmap, "another key"), hashmap_get(hashmap, "another key"));

  *str = '\0';
  strcat(str, "no");

  printf("another key (after messing with pointer): pointer = %p, value = %s\n", hashmap_get(hashmap, "another key"), hashmap_get(hashmap, "another key"));

  // void *replaced = hashmap_set(hashmap, "another key", &value1);
  // printf("another key (old value): pointer = %p, value = %s\n", replaced, replaced);
  // printf("another key (replaced): pointer = %p, value = %d\n", hashmap_get(hashmap, "another key"), *(int*) hashmap_get(hashmap, "another key"));

  printf("load factor = %f\n", hashmap_get_load_factor(hashmap));

  srand(time(NULL));
  for (size_t i = 0; i < 10000; i++) {
    char* key = alloca(20 * sizeof(char));
    sprintf(key, "key-%d", i);
    int* value = alloca(sizeof(int));
    *value = rand();

    if (i % 1000 == 0) {
      printf("key count = %d, load factor = %f\n", hashmap_count_keys(hashmap), hashmap_get_load_factor(hashmap));    }

    hashmap_set(hashmap, key, value);
  }

  size_t key_count = hashmap_count_keys(hashmap);

  printf("key count = %d\n", key_count);
  printf("load factor = %f\n", hashmap_get_load_factor(hashmap));

  printf("\nNow removing test keys...\n");
  char** keys = hashmap_get_keys(hashmap);
  for (size_t i = 0; i < key_count - 50; i++) {
    if (strcmp("another key", keys[i]) != 0 && strcmp("test key", keys[i]) != 0) {
      hashmap_del(hashmap, keys[i]);
    }
  }

  printf("test key: pointer = %p, value = %d\n", hashmap_get(hashmap, "test key"), *(int*) hashmap_get(hashmap, "test key"));
  printf("another key: pointer = %p, value = %s\n", hashmap_get(hashmap, "another key"), hashmap_get(hashmap, "another key"));
  printf("key count = %d\n", hashmap_count_keys(hashmap));
  printf("load factor = %f\n", hashmap_get_load_factor(hashmap));

  printf("Remaining keys:\n");
  key_count = hashmap_count_keys(hashmap);
  keys = hashmap_get_keys(hashmap);
  for (size_t i = 0; i < key_count; i++) {
    if (strcmp("another key", keys[i]) == 0) {
      printf("%s=%s\t", keys[i], hashmap_get(hashmap, keys[i]));
    } else {
      printf("%s=%d\t", keys[i], *(int*) hashmap_get(hashmap, keys[i]));
    }
  }
  printf("\n");


  hashmap_free(&hashmap);

  printf("\n\nMurmur3 sample hashes:\n");
  print_hash("");
  print_hash("hello, world");
  print_hash("hello, world!");
  print_hash("good bye, world!");
  print_hash("1");
  print_hash("2");
  print_hash("12");
  print_hash("true");

  return 0;
}
