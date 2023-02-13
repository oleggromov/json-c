#include <alloca.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse.h"
#include "dynarray.h"
#include "hashmap.h"
#include "util.h"

typedef struct {
  node_t* root;
  dynarray_t* node_stack;
  token_list_t* token_list;
  token_length_t next_token;
} parse_state_t;

static node_t* _allocate_node(node_type_t type, void* value)
{
  node_t* node = malloc(sizeof(node_t));
  node->type = type;
  node->value = value;
  return node;
}

static inline token_t* _next_token(parse_state_t* state)
{
  return &state->token_list->tokens[state->next_token];
}

static inline void _inc_next_token(parse_state_t* state)
{
  state->next_token++;
}

static inline void _append_node_object(parse_state_t* state)
{
  dynarray_append(state->node_stack, _allocate_node(NodeObject, hashmap_create()));
}

static inline void _append_node_array(parse_state_t* state)
{
  dynarray_append(state->node_stack, _allocate_node(NodeArray, dynarray_create()));
}

static void _consume_json_start(parse_state_t* state)
{
  if (_next_token(state)->type == TokenCurly) {
    _inc_next_token(state);
    _append_node_object(state);
    state->root = dynarray_get_top(state->node_stack);
    return;
  }

  if (_next_token(state)->type == TokenSquare) {
    _inc_next_token(state);
    _append_node_array(state);
    state->root = dynarray_get_top(state->node_stack);
    return;
  }

  die("parse: expected { or [ at JSON start"); // First token should be { or [
}

static void _consume_object_key_value(parse_state_t* state)
{
  char* key;
  size_t value_len;
  node_t* value_node;
  hashmap_t* hashmap_current = ((node_t*) dynarray_get_top(state->node_stack))->value;

  if (_next_token(state)->type == TokenString) {
    key = _next_token(state)->value_ptr;
    _inc_next_token(state);
  } else {
    die("parse: expected string as an object key");
  }

  if (_next_token(state)->type == TokenColon) {
    _inc_next_token(state);
  } else {
    die("parse: expected : after an object key");
  }

  // TODO who frees this pointer?
  value_node = _allocate_node(-1, NULL);

  // TODO who frees these pointers?
  switch(_next_token(state)->type) {
    case TokenCurly:
      _inc_next_token(state);
      _append_node_object(state);
      value_node->type = NodeObject;
      value_node->value = dynarray_get_top(state->node_stack);
      hashmap_set(hashmap_current, key, value_node);
      return; // don't consume a dangling comma

    case TokenSquare:
      _inc_next_token(state);
      _append_node_array(state);
      value_node->type = NodeArray;
      value_node->value = dynarray_get_top(state->node_stack);
      hashmap_set(hashmap_current, key, value_node);
      return; // don't consume a dangling comma

    case TokenString:
      value_len = strlen(_next_token(state)->value_ptr);
      value_node->type = NodeString;
      value_node->value = malloc(value_len * sizeof(char*));
      strncpy(value_node->value, _next_token(state)->value_ptr, value_len);
      _inc_next_token(state);
      break;

    case TokenLong:
      value_node->type = NodeLong;
      value_node->value = malloc(sizeof(long));
      *(long*) value_node->value = *(long*) _next_token(state)->value_ptr;
      _inc_next_token(state);
      break;

    case TokenDouble:
      value_node->type = NodeDouble;
      value_node->value = malloc(sizeof(double));
      *(double*) value_node->value = *(double*) _next_token(state)->value_ptr;
      _inc_next_token(state);
      break;

    case TokenBool:
      value_node->type = NodeBool;
      value_node->value = malloc(sizeof(int));
      *(int*) value_node->value = *(int*) _next_token(state)->value_ptr;
      _inc_next_token(state);
      break;

    case TokenNull:
      value_node->type = NodeNull;
      _inc_next_token(state);
      break;

    default:
      free(value_node);
      die("parse: unexpected token as an object key value");
  }

  hashmap_set(hashmap_current, key, value_node);

  if (_next_token(state)->type == TokenComma) {
    _inc_next_token(state);
  }
}

node_t* parse(token_list_t* token_list)
{
  parse_state_t state = {
    .node_stack = NULL,
    .next_token = 0,
    .token_list = token_list
  };

  if (token_list->length == 0) {
    return NULL;
  }

  state.node_stack = dynarray_create();

  while (state.next_token < state.token_list->length) {
    if (state.next_token == 0) {
      _consume_json_start(&state);
      continue;
    }

    if (state.node_stack->len == 0) {
      die("parse: unexpected token");
    }

    node_t* node_current = dynarray_get_top(state.node_stack);

    // Consuming object keys
    if (node_current->type == NodeObject) {
      if (_next_token(&state)->type == TokenUncurly) {
        dynarray_remove_top(state.node_stack);
        _inc_next_token(&state);

        if (state.node_stack->len > 0 && _next_token(&state)->type == TokenComma) {
          _inc_next_token(&state);
        }

        continue;
      }

      _consume_object_key_value(&state);
      continue;
    }

    // // Consume array values
    // if (node_current->type == )
  }

  if (state.node_stack->len != 0) {
    die("parse: expected more tokens");
  }

  dynarray_free(&state.node_stack);
  return state.root;
}

void free_node_list(node_t* node)
{

}

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
          sprintf(number_buf, "%d", *(long*) node_cur->value);
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
