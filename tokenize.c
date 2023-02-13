#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenize.h"
#include "util.h"

token_list_t* tokenize(char* str)
{
  token_list_t* result = allocate_token_list();
  result->tokens = NULL;
  result->length = 0;

  while (*str) {
    char error_buf[128];
    int* bool_value_ptr;
    int null_result;

    switch (*str) {
      case '{':
        append_token(result, TokenCurly);
        break;

      case '}':
        append_token(result, TokenUncurly);
        break;

      case '[':
        append_token(result, TokenSquare);
        break;

      case ']':
        append_token(result, TokenUnsquare);
        break;

      case ':':
        append_token(result, TokenColon);
        break;

      case ',':
        append_token(result, TokenComma);
        break;

      case '\"':
        append_token(result, TokenString);
        read_string(&str, (char**) &result->tokens[result->length - 1].value_ptr);
        break;

      case '.':
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        append_token(result, TokenLong);
        read_number(&str, &result->tokens[result->length - 1].value_ptr, &result->tokens[result->length - 1].type);
        break;

      case 't':
      case 'f':
        bool_value_ptr = read_bool(&str);
        if (NULL == bool_value_ptr) {
          sprintf(error_buf, "tokenize: expected true or false literal after \"%c\" in input", *str);
          die(error_buf);
        }

        append_token(result, TokenBool);
        result->tokens[result->length - 1].value_ptr = bool_value_ptr;
        break;

      case 'n':
        null_result = read_null(&str);
        if (-1 == null_result) {
          sprintf(error_buf, "tokenize: expected null literal after \"%c\" in input", *str);
          die(error_buf);
        }

        append_token(result, TokenNull);
        break;

      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;

      default:
        sprintf(error_buf, "tokenize: unexpected char \"%c\" in input", *str);
        die(error_buf);
    }

    str++;
  }

  return result;
}

static token_list_t* allocate_token_list()
{
  void *ptr = malloc(sizeof(token_list_t));
  if (ptr == NULL) {
    die("allocate_token_list: failed to allocated memory");
  }

  return ptr;
}

void free_token_list(token_list_t* token_list)
{
  for (token_length_t i = 0; i < token_list->length; i++) {
    free(token_list->tokens[i].value_ptr);
  }
  free(token_list->tokens);
}

// TODO allocate more in advance (estimate from the input length)
static token_t* reallocate_tokens(token_t* tokens, token_length_t length)
{
  void* ptr = realloc(tokens, length * sizeof(token_t));

  if (ptr == NULL) {
    die("reallocate_token: failed to allocate memory");
  }

  return ptr;
}

static void append_token(token_list_t* token_list, token_type_t token_type)
{
  token_list->tokens = reallocate_tokens(token_list->tokens, token_list->length + 1);
  token_list->tokens[token_list->length].type = token_type;
  token_list->tokens[token_list->length].value_ptr = NULL;
  token_list->length += 1;
}

static void read_string(char** str_ptr, char** read_str_ptr)
{
  // strings start with \"
  char* start = *str_ptr + 1;
  char* end = NULL;

  while (*(*str_ptr)++) {
    if (**str_ptr == '\n') {
      die("read_string: found \\n in a string");
    }

    if (**str_ptr == '\"') {
      end = *str_ptr;
      *read_str_ptr = malloc((end - start) + 1);
      **read_str_ptr = '\0';
      strncat(*read_str_ptr, start, end - start);
      return;
    }
  }

  die("read_string: found an unterminated string");
}

static void read_number(char** str_ptr, void** read_number_ptr, token_type_t* type_ptr)
{
  char* start = *str_ptr;
  char* end = NULL;

  int seen_minus = 0;
  int lead_digit = 0;
  int seen_period = 0;

  do {
    switch (**str_ptr) {
      // TODO:
      // exponent support
      // non 10- base support
      case '-':
        if (*str_ptr != start) {
          die("read_number: found extra \"-\" in a number literal");
        }

        seen_minus = 1;
        break;

      case '.':
        if (lead_digit == 0) {
          die("read_number: expected a digit in front of a \".\"");
        }

        if (seen_period == 1) {
          die("read_number: found extra \".\" in a number literal");
        }

        seen_period = 1;
        break;

      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
        if (start == *str_ptr || (seen_minus == 1 && start + 1 == *str_ptr)) {
          lead_digit = 1;
        }
        break;

      default:
        end = *str_ptr;
        // wind back one char
        *str_ptr -= 1;

        *read_number_ptr = malloc(seen_period ? sizeof(double) : sizeof(long));
        if (seen_period) {
          *(double*)*read_number_ptr = strtod(start, NULL);
          *type_ptr = TokenDouble;
        } else {
          *(long*)*read_number_ptr = strtol(start, NULL, 10);
          *type_ptr = TokenLong;
        }

        return;
    }
  } while (*(*str_ptr)++);
}

// Returns
// NULL if not a bool
// a pointer to allocated int 0/1 otherwise
static int* read_bool(char **str_ptr)
{
  if (0 == strncmp("true", *str_ptr, 4)) {
    *str_ptr += 3;
    int* value = malloc(sizeof(int));
    *value = 1;
    return value;
  }

  if (0 == strncmp("false", *str_ptr, 5)) {
    *str_ptr += 4;
    int* value = malloc(sizeof(int));
    *value = 0;
    return value;
  }

  return NULL;
}

// Returns
// 0 if null
// -1 otherwise
static int read_null(char **str_ptr)
{
  if (0 == strncmp("null", *str_ptr, 4)) {
    *str_ptr += 3;
    return 0;
  }

  return -1;
}

void print_token_list(token_t* token)
{
  const int PRINT_BUFFER_SIZE = 100;
  char buffer[PRINT_BUFFER_SIZE];

  switch (token->type)
  {
    case TokenCurly:
      printf("<CURLY> ");
      break;
    case TokenUncurly:
      printf("<UNCURLY> ");
      break;
    case TokenSquare:
      printf("<SQUARE> ");
      break;
    case TokenUnsquare:
      printf("<UNSQUARE> ");
      break;
    case TokenColon:
      printf("<COLON> ");
      break;
    case TokenComma:
      printf("<COMMA> ");
      break;
    case TokenString:
      strncpy(buffer, (char*) token->value_ptr, PRINT_BUFFER_SIZE - 1);
      printf("<STRING \"%s%s\"> ", buffer, strlen((char*) token->value_ptr) > PRINT_BUFFER_SIZE - 1 ? "..." : "");
      break;
    case TokenDouble:
      printf("<DOUBLE %f> ", *((double*) token->value_ptr));
      break;
    case TokenLong:
      printf("<LONG %d> ", *((int*) token->value_ptr));
      break;
    case TokenBool:
      printf("<BOOL %d> ", *((int*) token->value_ptr));
      break;
    case TokenNull:
      printf("<NULL> ");
      break;
  }
}
