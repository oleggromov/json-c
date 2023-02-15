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
  token_length_t token_current;
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

static inline token_t* _token_current(parse_state_t* state)
{
  return &state->token_list->tokens[state->token_current];
}

static inline void _inc_token_current(parse_state_t* state)
{
  // TODO check if it's out of bounds
  state->token_current++;
}

static node_t* _consume_value(parse_state_t* state)
{
  node_t* node = _allocate_node(0, NULL);
  token_t* token = _token_current(state);

  switch (token->type) {
    // Nested object or array
    case TokenCurly:
      node->type = NodeObject;
      node->value = hashmap_create();
      dynarray_append(state->node_stack, node);
      _inc_token_current(state);
      return node; // dangling comma isn't allowed

    case TokenSquare:
      node->type = NodeArray;
      node->value = dynarray_create();
      dynarray_append(state->node_stack, node);
      _inc_token_current(state);
      return node; // dangling comma isn't allowed

    // Primitive values
    case TokenString:
      node->type = NodeString;
      node->value = strdup(token->value_ptr);
      break;

    case TokenLong:
      node->type = NodeLong;
      node->value = malloc(sizeof(long));
      *(long*) node->value = *(long*) token->value_ptr;
      break;

    case TokenDouble:
      node->type = NodeDouble;
      node->value = malloc(sizeof(double));
      *(double*) node->value = *(double*) token->value_ptr;
      break;

    case TokenBool:
      node->type = NodeBool;
      node->value = malloc(sizeof(int));
      *(int*) node->value = *(int*) token->value_ptr;
      break;

    case TokenNull:
      node->type = NodeNull;
      break;

    default:
      // TODO better message
      die("parse: unexpected token at %d", token->_pos_start);
  }

  _inc_token_current(state);

  // dangling comma is allowed after a value
  if (_token_current(state)->type == TokenComma) {
    _inc_token_current(state);
  }

  return node;
}

node_t* parse(token_list_t* token_list, const char* input_str)
{
  parse_state_t state = {
    .root = NULL,
    .node_stack = dynarray_create(),
    .token_current = 0,
    .token_list = token_list
  };

  while (state.token_current < state.token_list->length) {

    node_t* node_top = NULL;

    if (state.token_current == 0) {
      if (_token_current(&state)->type == TokenCurly || _token_current(&state)->type == TokenSquare) {
        if (_token_current(&state)->type == TokenCurly) {
          dynarray_append(state.node_stack, _allocate_node(NodeObject, hashmap_create()));
        } else if (_token_current(&state)->type == TokenSquare) {
          dynarray_append(state.node_stack, _allocate_node(NodeArray, dynarray_create()));
        }

        _inc_token_current(&state);
        state.root = dynarray_get_top(state.node_stack);
        continue;
      }

      die("parse: expected { or [ at JSON start"); // First token should be { or [
    }

    if (state.node_stack->len == 0) {
      die(
        "parse: unexpected token at %lu\n",
        // "parse: unexpected token at %lu\n%s",
        _token_current(&state)->_pos_start
        // _slice_str_dangling_ptr(input_str, _next_token(&state)->_pos_start, _next_token(&state)->_pos_end - _next_token(&state)->_pos_start)
      );
    }

    node_top = dynarray_get_top(state.node_stack);

    // Consuming object key-value pairs
    if (node_top->type == NodeObject) {
      char* key = NULL;

      // Closing the current object
      if (_token_current(&state)->type == TokenUncurly) {
        dynarray_remove_top(state.node_stack);
        _inc_token_current(&state);

        // Dangling comma when it's not the last object
        if (state.node_stack->len > 0 && _token_current(&state)->type == TokenComma) {
          _inc_token_current(&state);
        }

        continue;
      }

      // Key
      if (_token_current(&state)->type == TokenString) {
        key = _token_current(&state)->value_ptr; // Hashmap internally makes a copy of the key
        _inc_token_current(&state);
      } else {
        die("parse: expected string as an object key");
      }

      // :
      if (_token_current(&state)->type == TokenColon) {
        _inc_token_current(&state);
      } else {
        die(
          "parse: expected : after an object key\n%s",
          _slice_str_dangling_ptr(input_str, _token_current(&state)->_pos_start, _token_current(&state)->_pos_end - _token_current(&state)->_pos_start)
        );
      }

      // And a value
      node_t* new_node = _consume_value(&state);

      // Finally, adding new node as a value for the consumed key
      hashmap_set((hashmap_t*) node_top->value, key, new_node);
      continue;
    }

    // Consuming array values
    if (node_top->type == NodeArray) {
      // Closing the current array
      if (_token_current(&state)->type == TokenUnsquare) {
        // DEBUG_dynarray_dump(dynarray_top);

        dynarray_remove_top(state.node_stack);
        _inc_token_current(&state);

        if (state.node_stack->len > 0 && _token_current(&state)->type == TokenComma) {
          _inc_token_current(&state);
        }

        continue;
      }

      // And a value
      node_t* new_node = _consume_value(&state);

      // printf("dynarray append addr = %p\n", node_val);
      dynarray_append((dynarray_t*) node_top->value, new_node);

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
