#ifndef READ_FILE_H
#define READ_FILE_H

#include <stdlib.h>

typedef enum {
  NotFound = 1025
} file_error_t;

typedef struct {
  char* buffer;
  size_t buffer_len;
  int error;
} text_file_t;

text_file_t* read_text_file(char* filename);
char* get_file_error(file_error_t error);

#endif
