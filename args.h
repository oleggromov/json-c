#ifndef ARGS_H
#define ARGS_H

#include <stdio.h>
#include <string.h>

typedef struct {
  char* input_file;
} json_args_t;

unsigned int get_args(json_args_t* json_args, int argc, char** argv);
void print_help();

#endif
