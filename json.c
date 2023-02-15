#include <stdlib.h>
#include <stdio.h>

#include "hashmap.h"
#include "tokenize.h"
#include "parse.h"

#include "dynarray.h"

// TODO fix "falsenull" or "truefalsefalse" cases
char* SAMPLE_INPUT = "{\"string_1\": \"a not so long string value\",\"object_1\": {\"number_float\": 3.14,\"number_int\": 327681, \"negative_number\": -0.231}\n, \"bool_true\": true, \"bool_false\": false, \"number\": 2.88, \"inner beauty\": [1, 2, 3, null]}";

char* NO_ARRAY = "{\"string_1\": \"a not so long string value\",\"object_1\": {\"number_float\": 3.14,\"number_int\": 327681, \"negative_number\": -0.231}\n, \"bool_true\": true, \"bool_false\": false, \"number\": 2.88 }";

char* SIMPLE = "{\"key_number\": 25.0303030303, \"key_string\": \"string\", \"boolean\": false, \"long\": 9200000 }";

char* SIMPLE_ARRAY = "[1,2,null,false,\"an example string\"]";

// 1. read file
// 2. split it into tokens
// 3. parse it into a tree
// 4. apply transformations
// 5. write it back to fs
int main()
{
  char* LOCAL_INPUT = SIMPLE_ARRAY;
  token_list_t* tokens = tokenize(LOCAL_INPUT);

  printf("\n\ntoken list length = %llu\n", tokens->length);
  if (tokens->length > 0) {
    printf("TOKENS:\n\n");
    for (token_length_t i = 0; i < tokens->length; i++) {
      print_token_list(&tokens->tokens[i]);
    }
    printf("\n");
  }

  node_t* root = parse(tokens, LOCAL_INPUT);
  printf("node root pointer = %p\n", (void*) root);

  // SIMPLE_ARRAY
  printf("root array[0] = %d\n", *(int*) ((node_t*) dynarray_get(root->value, 0))->value);
  printf("root array[1] = %d\n", *(int*) ((node_t*) dynarray_get(root->value, 1))->value);
  printf("root array[2] = %s\n", ((node_t*) dynarray_get(root->value, 2))->type == NodeNull ? "null" : "ERR!");
  printf("root array[3] = %d\n", ((node_t*) dynarray_get(root->value, 3))->type == NodeBool ? (int*) ((node_t*) dynarray_get(root->value, 2))->value : -1);
  printf("root array[4] = %s\n", ((node_t*) dynarray_get(root->value, 4))->value);

  // char* serialized = serialize(root);

  // printf("serialized:\n");
  // printf("%s\n", serialized);

  return EXIT_SUCCESS;
}
