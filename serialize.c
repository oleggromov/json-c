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


char* serialize(node_t* node)
{
  serialize_state_t state = {
    .buf = malloc(sizeof(char) * BUF_INCREMENT),
    .buf_pos = 0,
    .buf_len = BUF_INCREMENT
  };
  char number_buf[1024];

  if (node->type == NodeObject) {
    append_buf_str(&state, "{", 1);

    unsigned long obj_key_count = hashmap_count_keys(node->value);
    char** obj_keys = hashmap_get_keys(node->value);
    node_t* node_cur;

    for (unsigned long i = 0; i < obj_key_count; i++) {
      append_buf_str(&state, "\"", 1);
      append_buf_str(&state, obj_keys[i], strlen(obj_keys[i]));
      append_buf_str(&state, "\"", 1);
      append_buf_str(&state, ":", 1);

      node_cur = hashmap_get(node->value, obj_keys[i]);

      switch(node_cur->type) {
        case NodeString:
          append_buf_str(&state, "\"", 1);
          append_buf_str(&state, node_cur->value, strlen((char*) node_cur->value));
          append_buf_str(&state, "\"", 1);
          break;

        case NodeLong:
          sprintf(number_buf, "%ld", *(long*) node_cur->value);
          append_buf_str(&state, number_buf, strlen(number_buf));
          break;

        case NodeDouble:
          // TODO preserve original char* for serialisation
          sprintf(number_buf, "%g", *(double*) node_cur->value);
          append_buf_str(&state, number_buf, strlen(number_buf));
          break;

        case NodeBool:
          if (*(int*) node_cur->value == 1) {
            append_buf_str(&state, "true", 4);
          } else {
            append_buf_str(&state, "false", 5);
          }
          break;

        case NodeNull:
          append_buf_str(&state, "null", 4);
          break;
      }


      if (i < obj_key_count - 1) {
        append_buf_str(&state, ",", 1);
      }
    }

    append_buf_str(&state, "}", 1);
  }

  // terminating the string
  append_buf_str(&state, "\0", 1);

  return state.buf;
}
