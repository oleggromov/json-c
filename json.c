#include <stdlib.h>
#include <stdio.h>

#include "args.h"
#include "hashmap.h"
#include "tokenize.h"
#include "parse.h"
#include "serialize.h"
#include "dynarray.h"
#include "read_file.h"

int main(int argc, char** argv)
{
  json_args_t args;

  if (get_args(&args, argc, argv) == 1) {
    print_help();
    exit(EXIT_FAILURE);
  }

  text_file_t* input = read_text_file(args.input_file);
  if (input->error != 0) {
    fprintf(stderr, "Couldn't read file, error = %s\n", get_file_error(input->error));
    exit(EXIT_FAILURE);
  }

  // printf("Input file length = %zu\n", input->buffer_len);

  token_list_t* tokens = tokenize(input->buffer);

  printf("\n\ntoken list length = %llu\n", tokens->length);
  if (tokens->length > 0) {
    printf("TOKENS:\n\n");
    for (token_length_t i = 0; i < tokens->length; i++) {
      DEBUG_print_token(&tokens->tokens[i]);
    }
    printf("\n");
  }

  node_t* root = parse(tokens, input->buffer);

  char* serialized = serialize(root);
  printf("\n\nSERIALIZED:\n");
  printf("%s\n", serialized);

  return EXIT_SUCCESS;
}
