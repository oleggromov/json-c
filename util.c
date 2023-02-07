#include "util.h"

void die(char* message)
{
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}
