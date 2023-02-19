#include "args.h"

// Returns
// - 0 if read successfully
// - 1 if arguments are wrong
unsigned int get_args(json_args_t* json_args, int argc, char** argv)
{
  if (argc < 3) {
    return 1;
  }

  if (strncmp(argv[1], "-i", 2) != 0) {
    return 1;
  }

  json_args->input_file = argv[2];

  return 0;
}

void print_help(void)
{
  printf("Usage:\n");
  printf("json -i <input_file>\n");
}
