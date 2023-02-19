#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "args.h"
#include "hashmap.h"
#include "tokenize.h"
#include "parse.h"
#include "serialize.h"
#include "dynarray2.h"
#include "read_file.h"

int main(int argc, char** argv)
{
  clock_t start, end;
  json_args_t args;

  if (get_args(&args, argc, argv) == 1) {
    print_help();
    exit(EXIT_FAILURE);
  }

  start = clock();
  text_file_t* input = read_text_file(args.input_file);
  if (input->error != 0) {
    fprintf(stderr, "Couldn't read file, error = %s\n", get_file_error(input->error));
    exit(EXIT_FAILURE);
  }
  end = clock();
  printf("Reading file took: %f ms\n", ((double) end - start) / CLOCKS_PER_SEC * 1000);

  printf("Input file length = %zu\n", input->buffer_len);

  start = clock();
  dynarray2_t* tokens = tokenize(input->buffer);
  end = clock();

  printf("Tokenization took: %f ms\n", ((double) end - start) / CLOCKS_PER_SEC * 1000);
  printf("\n\ntoken list length = %zu\n", tokens->len);
  // if (tokens->len > 0) {
  //   printf("TOKENS:\n\n");
  //   for (size_t i = 0; i < tokens->len; i++) {
  //     DEBUG_print_token(dynarray2_get(tokens, i));
  //   }
  //   printf("\n");
  // }

  start = clock();
  node_t* root = parse(tokens, input->buffer);
  end = clock();
  printf("Parsing took: %f ms\n", ((double) end - start) / CLOCKS_PER_SEC * 1000);

  start = clock();
  char* serialized = serialize(root);
  end = clock();
  printf("Serialization took: %f ms\n", ((double) end - start) / CLOCKS_PER_SEC * 1000);
  // printf("\n\nSERIALIZED:\n");
  // printf("%s\n", serialized);

  return EXIT_SUCCESS;
}
