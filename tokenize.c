#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenize.h"
#include "util.h"


// Internal functions
static token_list_t* allocate_token_list();
static token_t* reallocate_tokens(token_t* tokens, token_length_t length);

static void append_token(token_list_t* token_list, token_type_t token_type, size_t pos_start);

static void read_string(char** str_ptr, char** read_str_ptr);
static void read_number(char** str_ptr, void** read_number_ptr, token_type_t* type_ptr);
static int* read_bool(char** str_ptr);
static int read_null(char** str_ptr);

// TODO:
// - should the tokenizer know about delimiters (",}]") after values? (falsenull or "string""string" and other things)
// - testcase: fill memory with garbage and check what tokenize returns
token_list_t* tokenize(const char* str)
{
  token_list_t* result = allocate_token_list();
  result->tokens = NULL;
  result->length = 0;

  char* cur = (char*) str;
  size_t tmp_pos;

  while (*cur) {
    int* bool_value_ptr;
    int null_result;

    switch (*cur) {
      case '{':
        append_token(result, TokenCurly, cur - str);
        break;

      case '}':
        append_token(result, TokenUncurly, cur - str);
        break;

      case '[':
        append_token(result, TokenSquare, cur - str);
        break;

      case ']':
        append_token(result, TokenUnsquare, cur - str);
        break;

      case ':':
        append_token(result, TokenColon, cur - str);
        break;

      case ',':
        append_token(result, TokenComma, cur - str);
        break;

      case '\"':
        append_token(result, TokenString, cur - str);
        read_string(&cur, (char**) &result->tokens[result->length - 1].value_ptr);
        result->tokens[result->length-1]._pos_end = cur - str;
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
        append_token(result, TokenLong, cur - str);
        read_number(&cur, &result->tokens[result->length - 1].value_ptr, &result->tokens[result->length - 1].type);
        result->tokens[result->length-1]._pos_end = cur - str;
        break;

      // TODO fix falsenull and other insanities
      case 't':
      case 'f':
        tmp_pos = cur - str;

        bool_value_ptr = read_bool(&cur);
        if (NULL == bool_value_ptr) {
          die("tokenize: expected true or false literal after \"%c\" in input", *cur);
        }

        append_token(result, TokenBool, tmp_pos);
        result->tokens[result->length - 1].value_ptr = bool_value_ptr;
        result->tokens[result->length-1]._pos_end = cur - str;
        break;

      case 'n':
        tmp_pos = cur;

        null_result = read_null(&cur);
        if (-1 == null_result) {
          die("tokenize: expected null literal after \"%c\" in input", *cur);
        }

        append_token(result, TokenNull, tmp_pos);
        result->tokens[result->length-1]._pos_end = cur - str;
        break;

      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;

      default:
        die("tokenize: unexpected char \"%c\" in input", *cur);
    }

    cur++;
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

// Internal functions
static void append_token(token_list_t* token_list, token_type_t token_type, size_t pos_start)
{
  token_list->tokens = reallocate_tokens(token_list->tokens, token_list->length + 1);
  token_list->tokens[token_list->length].type = token_type;
  token_list->tokens[token_list->length].value_ptr = NULL;
  token_list->tokens[token_list->length]._pos_start = pos_start;
  token_list->length += 1;
}

// Comes null terminated
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
      *read_str_ptr = malloc(((end - start) + 1) * sizeof(char));
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
    // TODO: will this potentially cause a buffer overflow?
    *str_ptr += 3;
    return 0;
  }

  return -1;
}

void DEBUG_print_token(token_t* token)
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
      printf("<DOUBLE %g> ", *((double*) token->value_ptr));
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
