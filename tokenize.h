#ifndef TOKENIZE_H
#define TOKENIZE_H

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
  char* _orig_str;
  size_t _orig_len;
} token_t;

typedef unsigned long long token_length_t;

typedef struct {
  token_length_t length;
  token_t* tokens;
} token_list_t;

token_list_t* tokenize(char* str);
void free_token_list(token_list_t* token_list);

void print_token_list(token_t* token);

#endif //TOKENIZE_H
