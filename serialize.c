#include "alloca.h"
#include "serialize.h"

typedef struct {
  char* buf;
  unsigned long buf_pos;
  unsigned long buf_len;
} serialize_state_t;

static const size_t BUF_INCREMENT = 1024;
static void append_buf_str(serialize_state_t* state, char* str, unsigned long str_len)
{
  // example: 1023 + 0 == 1024 - 1
  if (state->buf_pos + str_len >= state->buf_len - 1) {
    unsigned long buf_ext = ((str_len - (state->buf_len - state->buf_pos)) / BUF_INCREMENT + 1) * BUF_INCREMENT;

    state->buf_len += buf_ext;
    state->buf = realloc(state->buf, sizeof(char) * state->buf_len);
    if (state->buf == NULL) {
      die("append_buf_str: reallocation failed");
    }
  }

  strncpy(state->buf + state->buf_pos, str, str_len);
  state->buf_pos += str_len;
}

static void append_buf_indent(serialize_state_t* state, int depth)
{
  if (depth > 0) {
    char* indent_str = alloca(sizeof(char) * depth);
    memset(indent_str, '\t', depth);
    append_buf_str(state, indent_str, depth);
  }
}

static void append_buf_str_indented(serialize_state_t* state, char* str, unsigned long str_len, unsigned int depth) {
  append_buf_str(state, "\n", 1);
  append_buf_indent(state, depth);
  append_buf_str(state, str, str_len);
}

static void step(serialize_state_t* state, node_t* node, unsigned int depth, bool indent_values)
{
  size_t arr_len = -1;
  size_t obj_key_count = -1;
  char** obj_keys = NULL;
  char number_buf[1024] = {0};

  switch (node->type) {
    case NodeObject:
      if (indent_values) {
        append_buf_str_indented(state, "{", 1, depth);
      } else {
        append_buf_str(state, "{", 1);
      }

      obj_key_count = hashmap_count_keys(node->value);
      obj_keys = hashmap_get_keys(node->value);

      for (size_t i = 0; i < obj_key_count; i++) {
        append_buf_str_indented(state, "\"", 1, depth + 1);
        append_buf_str(state, obj_keys[i], strlen(obj_keys[i]));
        append_buf_str(state, "\"", 1);
        append_buf_str(state, ": ", 2);

        step(state, hashmap_get(node->value, obj_keys[i]), depth + 1, false);

        if (i < obj_key_count - 1) {
          append_buf_str(state, ", " , 2);
        }
      }

      append_buf_str_indented(state, "}", 1, depth);
      break;

    case NodeArray:
      append_buf_str(state, "[", 1);

      arr_len = ((dynarray_t*) node->value)->len;

      for (size_t i = 0; i < arr_len; i++) {
        node_t* child = dynarray_get(node->value, i);

        step(state, child, depth + 1, true);

        if (i < arr_len - 1) {
          append_buf_str(state, ", " , 2);
        }
      }

      append_buf_str_indented(state, "]", 1, depth);
      break;

    case NodeString:
      if (indent_values) {
        append_buf_str_indented(state, "\"", 1, depth);
      } else {
        append_buf_str(state, "\"", 1);
      }
      append_buf_str(state, node->value, strlen((char*) node->value));
      append_buf_str(state, "\"", 1);
      break;

    case NodeLong:
      sprintf(number_buf, "%ld", *(long*) node->value);
      if (indent_values) {
        append_buf_str_indented(state, number_buf, strlen(number_buf), depth);
      } else {
        append_buf_str(state, number_buf, strlen(number_buf));
      }
      break;

    case NodeDouble:
      // TODO preserve original char* for serialisation
      sprintf(number_buf, "%g", *(double*) node->value);
      if (indent_values) {
        append_buf_str_indented(state, number_buf, strlen(number_buf), depth);
      } else {
        append_buf_str(state, number_buf, strlen(number_buf));
      }
      break;

    case NodeBool:
      if (*(int*) node->value == 1) {
        if (indent_values) {
          append_buf_str_indented(state, "true", 4, depth);
        } else {
          append_buf_str(state, "true", 4);
        }
      } else {
        if (indent_values) {
          append_buf_str_indented(state, "false", 5, depth);
        } else {
          append_buf_str(state, "false", 5);
        }
      }
      break;

    case NodeNull:
      if (indent_values) {
        append_buf_str_indented(state, "null", 4, depth);
      } else {
        append_buf_str(state, "null", 4);
      }
      break;
  }
}

char* serialize(node_t* node) {
  serialize_state_t state = {
    .buf = malloc(sizeof(char) * BUF_INCREMENT),
    .buf_pos = 0,
    .buf_len = BUF_INCREMENT
  };

  step(&state, node, 0, false);

  append_buf_str(&state, "\0", 1);
  return state.buf;
}



// char* serialize_iterative(node_t* node)
// {
//   serialize_state_t state = {
//     .buf = malloc(sizeof(char) * BUF_INCREMENT),
//     .buf_pos = 0,
//     .buf_len = BUF_INCREMENT
//   };
//   char number_buf[1024];

//   dynarray_t* stack = dynarray_create();
//   dynarray_t* unclosed = dynarray_create();
//   dynarray_append(stack, node);

//   while (stack->len > 0) {
//     // printf("len = %d\n", (int) stack->len);
//     node_t* cur = dynarray_remove_top(stack);
//     size_t obj_key_count;
//     char** obj_keys;
//     node_t* fake_node = NULL;

//     switch (cur->type) {
//       case NodeObject:
//         append_buf_str(&state, "{", 1);
//         dynarray_append(unclosed, cur);

//         obj_key_count = hashmap_count_keys(cur->value);
//         obj_keys = hashmap_get_keys(cur->value);

//         for (size_t i = 0; i < obj_key_count; i++) {
//           // append_buf_str(&state, "\"", 1);
//           // append_buf_str(&state, obj_keys[i], strlen(obj_keys[i]));
//           // append_buf_str(&state, "\"", 1);
//           // append_buf_str(&state, ":", 1);

//           // fake_node = malloc(sizeof(node_t));
//           // fake_node->type = NodeString;
//           // fake_node->value = obj_keys[i];
//           dynarray_append(stack, hashmap_get(cur->value, obj_keys[i]));
//           // printf("stack len = %d, append type = %d\n", stack->len, ((node_t*) hashmap_get(cur->value, obj_keys[i]))->type);
//           // dynarray_append(stack, fake_node);
//         }
//         break;

//       case NodeArray:
//         append_buf_str(&state, "[", 1);
//         dynarray_append(unclosed, cur);

//         for (size_t i = 0; i < ((dynarray_t*) cur->value)->len; i++) {
//           dynarray_append(stack, dynarray_get(cur->value, i));
//         }
//         break;

//       case NodeString:
//         append_buf_str(&state, "\"", 1);
//         append_buf_str(&state, cur->value, strlen((char*) cur->value));
//         append_buf_str(&state, "\"", 1);
//         break;

//       case NodeLong:
//         sprintf(number_buf, "%ld", *(long*) cur->value);
//         append_buf_str(&state, number_buf, strlen(number_buf));
//         break;

//       case NodeDouble:
//         // TODO preserve original char* for serialisation
//         sprintf(number_buf, "%g", *(double*) cur->value);
//         append_buf_str(&state, number_buf, strlen(number_buf));
//         break;

//       case NodeBool:
//         if (*(int*) cur->value == 1) {
//           append_buf_str(&state, "true", 4);
//         } else {
//           append_buf_str(&state, "false", 5);
//         }
//         break;

//       case NodeNull:
//         append_buf_str(&state, "null", 4);
//         break;
//     }

//     append_buf_str(&state, ", ", 2);
//   }

//   while (unclosed->len > 0) {
//     node_t* cur = dynarray_remove_top(unclosed);

//     switch (cur->type) {
//       case NodeObject:
//         append_buf_str(&state, "}", 1);
//         break;

//       case NodeArray:
//         append_buf_str(&state, "]", 1);
//         break;
//     }
//   }

//   // terminating the string
//   append_buf_str(&state, "\0", 1);

//   return state.buf;
// }
