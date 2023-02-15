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

static char* _slice_str_dangling_ptr(const char* str, size_t offset, size_t min_len)
{
  const size_t PADDING = 0;
  // 24 = 4, 4 = 0
  size_t print_start = offset >= PADDING ? offset - PADDING : 0;
  size_t str_len = strlen(str);
  size_t print_end = offset + min_len + PADDING < str_len ? str_len - (offset + min_len + PADDING) : str_len;

  char* slice = malloc(sizeof(char) * (3+3+1 + print_end - print_start));
  *slice = '\0';

  if (print_start > 0) {
    strncat(slice, "...", 3);
  }

  strncat(slice, str + print_start, print_end - print_start);

  if (print_end < str_len - 1) {
    strncat(slice, "...", 3);
  }

  return slice;
}

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

static void _consume_object_key_value(parse_state_t* state, const char* input_str)
{
  char* key;
  size_t value_len;
  node_t* value_node;
  hashmap_t* hashmap_current = ((node_t*) dynarray_get_top(state->node_stack))->value;

  if (_next_token(state)->type == TokenString) {
    // TODO this is potentially freed by freeing token_list_t
    key = _next_token(state)->value_ptr;
    _inc_next_token(state);
  } else {
    die("parse: expected string as an object key");
  }

  if (_next_token(state)->type == TokenColon) {
    _inc_next_token(state);
  } else {
    die(
      "parse: expected : after an object key\n%s",
      _slice_str_dangling_ptr(input_str, _next_token(state)->_pos_start, _next_token(state)->_pos_end - _next_token(state)->_pos_start)
    );
  }

  // TODO who frees this pointer?
  value_node = _allocate_node(-1, NULL);

  // TODO who frees these pointers?
  switch(_next_token(state)->type) {
    // case TokenUncurly:
    //   dynarray_remove_top(state->node_stack);
    //   _inc_next_token(state);

    case TokenCurly:
      _inc_next_token(state);
      _append_node_object(state);
      value_node->value = dynarray_get_top(state->node_stack);
      hashmap_set(hashmap_current, key, value_node);
      return; // don't consume a dangling comma

    case TokenSquare:
      _inc_next_token(state);
      _append_node_array(state);
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

  // dangling comma is allowed
  if (_next_token(state)->type == TokenComma) {
    _inc_next_token(state);
  }
}

static void _consume_array_values(parse_state_t* state) {
  size_t value_len;
  // TODO who frees this pointer?
  node_t* value_node = _allocate_node(-1, NULL);
  dynarray_t* array_current = ((node_t*) dynarray_get_top(state->node_stack))->value;

  switch (_next_token(state)->type) {
    case TokenCurly:
      _inc_next_token(state);
      _append_node_object(state);
      value_node->value = dynarray_get_top(state->node_stack);
      dynarray_append(state->node_stack, value_node);
      return; // don't consume a dangling comma

    case TokenSquare:
      _inc_next_token(state);
      _append_node_array(state);
      value_node->value = dynarray_get_top(state->node_stack);
      dynarray_append(state->node_stack, value_node);
      return; // don't consume dangling comma

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
      die("parse: unexpected token as an array value");
  }

  dynarray_append(array_current, value_node);

  if (_next_token(state)->type == TokenComma) {
    _inc_next_token(state);
  }
}

node_t* parse(token_list_t* token_list, const char* input_str)
{
  parse_state_t state = {
    .root = NULL,
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
      die(
        "parse: unexpected token at %lu\n%s",
        _next_token(&state)->_pos_start,
        _slice_str_dangling_ptr(input_str, _next_token(&state)->_pos_start, _next_token(&state)->_pos_end - _next_token(&state)->_pos_start)
      );
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

      _consume_object_key_value(&state, input_str);
      continue;
    }

    // Consuming array values
    if (node_current->type == NodeArray) {
      if (_next_token(&state)->type == TokenUnsquare) {
        dynarray_remove_top(state.node_stack);
        _inc_next_token(&state);

        if (state.node_stack->len > 0 && _next_token(&state)->type == TokenComma) {
          _inc_next_token(&state);
        }

        continue;
      }

      _consume_array_values(&state);
      continue;
    }
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
