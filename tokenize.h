#ifndef TOKENIZE_H
#define TOKENIZE_H

enum TokenType { Curly, Uncurly, Colon, Comma, String, Double, Long };

struct Token {
  enum TokenType type;
  void* value_ptr;
};

struct TokenList {
  unsigned long long length;
  struct Token* tokens;
};

struct TokenList* tokenize(char* str);
void free_token_list(struct TokenList* token_list);

void print_token_list(struct Token* token);

static struct TokenList* allocate_token_list();
static struct Token* reallocate_tokens(struct Token* tokens, unsigned long long size);

static void append_token(struct TokenList* token_list, int token_type);

static void read_string(char** str_ptr, char** read_str_ptr);
static void read_number(char** str_ptr, void** read_number_ptr, enum TokenType* type_ptr);

#endif //TOKENIZE_H
