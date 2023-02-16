#ifndef SERIALIZE_H
#define SERIALIZE_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

#include "parse.h"
#include "dynarray.h"
#include "hashmap.h"
#include "util.h"

char* serialize(node_t* node);

#endif
