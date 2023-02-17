#ifndef TOKENIZE_H
#define TOKENIZE_H

#include "dynarray2.h"

typedef enum {
  TokenCurly, TokenUncurly, TokenSquare, TokenUnsquare,
  TokenColon, TokenComma,
  TokenString, TokenDouble, TokenLong, TokenBool, TokenNull,
} token_type_t;

typedef struct {
  token_type_t type;
  void* value_ptr;

  // TODO add extra info
  size_t _pos_start;
  size_t _pos_end;
} token_t;

typedef unsigned long long token_length_t;

dynarray2_t* tokenize(const char* str);

void DEBUG_print_token(token_t* token);

#endif //TOKENIZE_H
