#ifndef PARSE_H
#define PARSE_H

#include "tokenize.h"

typedef enum {
  NodeObject = 255, NodeArray,
  NodeString, NodeLong, NodeDouble, NodeBool, NodeNull
} node_type_t;

typedef struct {
  node_type_t type;
  void* value;
} node_t;

node_t* parse(token_list_t* token_list, const char* input_str);
void free_node_list(node_t* node);

#endif //PARSE_H
