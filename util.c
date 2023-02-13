#include "util.h"

void die(char* message, ...)
{
  char format[1024] = {0};
  strcat(&format, message);
  strcat(&format, "\n");

  va_list args;
  va_start(args, message);
  vfprintf(stderr, format, args);
  va_end(args);

  exit(EXIT_FAILURE);
}
