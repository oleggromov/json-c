#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <float.h>

#include "tokenize.h"
#include "util.h"

// Internal functions
static void append_token(dynarray2_t* token_list, token_type_t token_type, size_t pos_start);

static void read_string(char** str_ptr, char** read_str_ptr);
static void read_number(char** str_ptr, void** read_number_ptr, token_type_t* type_ptr);
static int* read_bool(char** str_ptr);
static int read_null(char** str_ptr);

// TODO:
// - \C escaped symbols
// - 1e25 number notation
// - should the tokenizer know about delimiters (",}]") after values? (falsenull or "string""string" and other things)
// - testcase: fill memory with garbage and check what tokenize returns
dynarray2_t* tokenize(const char* str)
{
  dynarray2_t* token_list = dynarray2_create(sizeof(token_t));
  token_t* current_token = NULL;

  char* cur = (char*) str;
  ptrdiff_t tmp_pos;

  char err_buf[1024] = {0};

  while (*cur) {
    int* bool_value_ptr;
    int null_result;

    switch (*cur) {
      case '{':
        append_token(token_list, TokenCurly, cur - str);
        break;

      case '}':
        append_token(token_list, TokenUncurly, cur - str);
        break;

      case '[':
        append_token(token_list, TokenSquare, cur - str);
        break;

      case ']':
        append_token(token_list, TokenUnsquare, cur - str);
        break;

      case ':':
        append_token(token_list, TokenColon, cur - str);
        break;

      case ',':
        append_token(token_list, TokenComma, cur - str);
        break;

      case '\"':
        append_token(token_list, TokenString, cur - str);
        current_token = dynarray2_get_top(token_list);
        read_string(&cur, (char**) &current_token->value_ptr);
        current_token->_pos_end = cur - str;
        current_token = NULL;
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
        append_token(token_list, TokenLong, cur - str);
        current_token = dynarray2_get_top(token_list);
        read_number(&cur, &current_token->value_ptr, &current_token->type);
        current_token->_pos_end = cur - str;
        current_token = NULL;
        break;

      // TODO fix falsenull and other insanities
      case 't':
      case 'f':
        tmp_pos = cur - str;

        bool_value_ptr = read_bool(&cur);
        if (NULL == bool_value_ptr) {
          die("tokenize: expected true or false literal after \"%c\" in input", *cur);
        }

        append_token(token_list, TokenBool, tmp_pos);
        current_token = dynarray2_get_top(token_list);
        current_token->value_ptr = bool_value_ptr;
        current_token->_pos_end = cur - str;
        current_token = NULL;
        break;

      case 'n':
        tmp_pos = (ptrdiff_t) cur;

        null_result = read_null(&cur);
        if (-1 == null_result) {
          die("tokenize: expected null literal after \"%c\" in input in position %d", *cur, cur - str);
        }

        append_token(token_list, TokenNull, tmp_pos);
        current_token = dynarray2_get_top(token_list);
        current_token->_pos_end = cur - str;
        break;

      case ' ':
      case '\n':
      case '\r':
      case '\t':
        break;

      default:
        strncpy(err_buf, (str + (cur - str - 20)), 41);
        die("tokenize: unexpected char \"%c\" in input in position %d\n%s", *cur, cur - str, err_buf);
    }

    cur++;
  }

  return token_list;
}

// Internal functions
static void append_token(dynarray2_t* token_list, token_type_t token_type, size_t pos_start)
{
  token_t value = {
    .type = token_type,
    .value_ptr = NULL,
    ._pos_start = pos_start
  };
  dynarray2_append(token_list, &value);
}


static const char ESCAPE_SYMBOLS[] = {'"', '\\', '/', 'b', 'f', 'n', 'r', 't', 'u'};


// TODO use isxdigit
// static const char HEX_DIGITS[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'};

// TODO add \uXXXX hex validation
static inline bool _is_valid_escape_char(char c)
{
  for (size_t i = 0; i < sizeof(ESCAPE_SYMBOLS); i++) {
    if (c == ESCAPE_SYMBOLS[i]) {
      return true;
    }
  }
  return false;
}

// Returned null terminated
static void read_string(char** str_ptr, char** read_str_ptr)
{
  // strings start with \"
  char* start = *str_ptr + 1;
  char* end = NULL;
  bool is_escape = false;

  while (*(*str_ptr)++) {
    if (is_escape) {
      if (_is_valid_escape_char(**str_ptr)) {
        is_escape = false;
        continue;
      }

      die("tokenize/read_string: unexpected escape sequence \\%c", **str_ptr);
    }

    if (**str_ptr == '\\') {
      is_escape = true;
      continue;
    }

    if (**str_ptr == '\"') {
      end = *str_ptr;
      *read_str_ptr = malloc(((end - start) + 1) * sizeof(char));
      **read_str_ptr = '\0';
      strncat(*read_str_ptr, start, end - start);
      return;
    }
  }

  die("tokenize/read_string: found an unterminated string");
}

static void read_number(char** str_ptr, void** read_number_ptr, token_type_t* type_ptr)
{
  char* start = *str_ptr;
  char* end = NULL;

  bool seen_minus = 0;
  bool lead_digit = 0;
  bool seen_period = 0;

  do {
    switch (**str_ptr) {
      // TODO:
      // - scientific notation
      // - big ints
      case '-':
        if (*str_ptr != start) {
          die("read_number: found extra \"-\" in a number literal");
        }

        seen_minus = true;
        break;

      case '.':
        if (lead_digit == false) {
          die("read_number: expected a digit in front of a \".\"");
        }

        if (seen_period == true) {
          die("read_number: found extra \".\" in a number literal");
        }

        seen_period = true;
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
          lead_digit = true;
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
