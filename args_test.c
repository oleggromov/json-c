#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "args.h"
#include "testlib.h"

static void args_any_order()
{
  int ret;
  json_args_t res;

  char* argv[] = {"./json", "-i", "test-input.txt", "-o", "/tmp/test-output.txt"};
  ret = get_args(&res, 5, argv);
  assert(ret == 0);
  assert(strcmp(res.input_file, "test-input.txt") == 0);
  assert(strcmp(res.output_file, "/tmp/test-output.txt") == 0);

  char *argv2[] = {"./any-binary", "-o", "output.json", "-i", "./path/input.json"};
  ret = get_args(&res, 5, argv2);
  assert(ret == 0);
  assert(strcmp(res.input_file, "./path/input.json") == 0);
  assert(strcmp(res.output_file, "output.json") == 0);
}

static void less_args()
{
  int ret;
  json_args_t res = {NULL, NULL};
  char* argv[] = {"./json", "-i", "test-input.txt"};

  ret = get_args(&res, 3, argv);
  assert(ret == 1);
  assert(res.input_file == NULL);
  assert(res.output_file == NULL);
}

static void more_args()
{
int ret;
  json_args_t res;
  char* argv[] = {"./json", "-i", "test-input.txt", "-o", "/tmp/test-output.txt", "-test", "something"};

  ret = get_args(&res, 7, argv);
  assert(ret == 0);
  assert(strcmp(res.input_file, "test-input.txt") == 0);
  assert(strcmp(res.output_file, "/tmp/test-output.txt") == 0);

  char *argv2[] = {"./any-binary", "-o", "output.json", "-i", "./path/input.json", "something-file.json"};
  ret = get_args(&res, 6, argv2);
  assert(ret == 0);
  assert(strcmp(res.input_file, "./path/input.json") == 0);
  assert(strcmp(res.output_file, "output.json") == 0);
}

static void* b() { return NULL; }
static void a() {}

int main(void)
{
  test("required arguments in any order work", args_any_order, b, a);
  test("less arguments work", less_args, b, a);
  test("more arguments work", more_args, b, a);
}
