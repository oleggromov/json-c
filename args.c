#include <stdio.h>
#include <string.h>

#include "args.h"

// Returns
// - 0 if read successfully
// - 1 if arguments are wrong
unsigned int get_args(json_args_t* json_args, int argc, char** argv)
{
  const int PAIRS = 2;
  struct {
    char* key;
    char* value;
  } READ[] = {
    {"-i", NULL},
    {"-o", NULL}
  };

  int found = 0;
  int arg_i = 1;

  if (argc >= PAIRS * 2 + 1) {
    while (arg_i < argc && found < PAIRS) {
      for (int read_i = 0; read_i < PAIRS; read_i++) {
        if (strncmp(argv[arg_i], READ[read_i].key, strlen(READ[read_i].key)) == 0) {
          READ[read_i].value = argv[++arg_i];
          found++;
        }
      }
      arg_i++;
    }

    if (found == PAIRS) {
      json_args->input_file = READ[0].value;
      json_args->output_file = READ[1].value;
      return 0;
    }
  }

  return 1;
}

void print_help(void)
{
  printf("Usage:\n");
  printf("json -i <input_file> -o <output_file>\n");
}
