#include <stdlib.h>
#include <stdio.h>
#include "tokenize.h"

char* SAMPLE_INPUT = "{\"string_1\": \"a not so long string value\",\"object_1\": {\"number_float\": 3.14,\"number_int\": 327681,, \"negative_number\": -0.231}\n}";

// 1. read file
// 2. split it into tokens
// 3. parse it into a tree
// 4. apply transformations
// 5. write it back to fs
int main()
{
  struct TokenList* tokens = tokenize(SAMPLE_INPUT);

  printf("\n\ntoken list length = %llu\n", tokens->length);
  if (tokens->length > 0) {
    printf("TOKENS:\n\n");
    for (unsigned long long i = 0; i < tokens->length; i++) {
      print_token_list(&tokens->tokens[i]);
    }
    printf("\n");
  }

  free_token_list(tokens);

  return EXIT_SUCCESS;
}
