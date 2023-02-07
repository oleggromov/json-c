#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "tokenize.h"
#include "util.h"

struct TokenList* tokenize(char* str)
{
  struct TokenList* result = allocate_token_list();
  result->tokens = NULL;
  result->length = 0;

  char error_buf[128];

  while (*str) {
    switch (*str) {
      case '{':
        append_token(result, Curly);
        break;

      case '}':
        append_token(result, Uncurly);
        break;

      case ':':
        append_token(result, Colon);
        break;

      case ',':
        append_token(result, Comma);
        break;

      case '\"':
        append_token(result, String);
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
        append_token(result, Long);
        read_number(&str, &result->tokens[result->length - 1].value_ptr, &result->tokens[result->length - 1].type);
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

static struct TokenList* allocate_token_list()
{
  return malloc(sizeof(struct TokenList));
}

void free_token_list(struct TokenList* token_list)
{
  for (unsigned long long i = 0; i < token_list->length; i++) {
    free(token_list->tokens[i].value_ptr);
  }
  free(token_list->tokens);
}

static struct Token* reallocate_tokens(struct Token* tokens, unsigned long long size)
{
  void* ptr = realloc(tokens, size * sizeof(struct Token));

  if (ptr == NULL) {
    die("reallocate_token: failed to allocate memory");
  }

  return ptr;
}

static void append_token(struct TokenList* token_list, int token_type)
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

static void read_number(char** str_ptr, void** read_number_ptr, enum TokenType* type_ptr)
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
          *type_ptr = Double;
        } else {
          *(long*)*read_number_ptr = strtol(start, NULL, 10);
          *type_ptr = Long;
        }

        return;
    }
  } while (*(*str_ptr)++);
}

const int PRINT_BUFFER_SIZE = 100;
void print_token_list(struct Token* token)
{
  char buffer[PRINT_BUFFER_SIZE];

  switch (token->type)
  {
    case Curly:
      printf("<CURLY> ");
      break;
    case Uncurly:
      printf("<UNCURLY> ");
      break;
    case Colon:
      printf("<COLON> ");
      break;
    case Comma:
      printf("<COMMA> ");
      break;
    case String:
      strncpy(buffer, (char*) token->value_ptr, PRINT_BUFFER_SIZE - 1);
      printf("<STRING \"%s%s\"> ", buffer, strlen((char*) token->value_ptr) > PRINT_BUFFER_SIZE - 1 ? "..." : "");
      break;
    case Double:
      printf("<DOUBLE %f> ", *((double*) token->value_ptr));
      break;
    case Long:
      printf("<LONG %d> ", *((int*) token->value_ptr));
      break;
  }
}
