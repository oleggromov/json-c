#include "read_file.h"

#include <stdio.h>

text_file_t* read_text_file(char* filename)
{
  text_file_t* result = malloc(sizeof(text_file_t));

  result->buffer = NULL;
  result->buffer_len = 0;
  result->error = 0;

  FILE* fp = NULL;
  fp = fopen(filename, "r");

  if (fp == NULL) {
    result->error = NotFound;
    fclose(fp);
    return result;
  }

  fseek(fp, 0, SEEK_END);
  result->buffer_len = ftell(fp) + 1; // for the \0
  fseek(fp, 0, SEEK_SET);

  result->buffer = malloc(sizeof(char) * result->buffer_len);

  fread(result->buffer, sizeof(char), result->buffer_len - 1, fp);
  result->buffer[result->buffer_len-1] = '\0';

  if ((result->error = ferror(fp)) != 0) {
    free(result->buffer);
    result->buffer = NULL;
  }

  fclose(fp);
  return result;
}

char* get_file_error(file_error_t error)
{
  switch (error) {
    case NotFound:
      return "File not found";

    default:
      return "Unknown file read error";
  }
}
