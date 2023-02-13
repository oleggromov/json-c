#ifndef PARSE_H
#define PARSE_H

#include "tokenize.h"

typedef enum {
  NodeObject, NodeArray,
  NodeString, NodeLong, NodeDouble, NodeBool, NodeNull
} node_type_t;

typedef struct {
  node_type_t type;
  void* value;
} node_t;

node_t* parse(token_list_t* token_list);
void free_node_list(node_t* node);

char* serialize(node_t* node);

// Object
// - keys:
// -

// {
//   "string_1": "a not so long string value",
//   "object_1": {
//     "number_float": 0.14,
//     "number_int": 32768
//   },
//   "array": [
//     {
//       "object": true,
//       "sub_object": {
//         "inner_array": [1, 2, 3, "string"]
//       }
//     },
//     "another string",
//     false
//   ]
// }


#endif //PARSE_H
