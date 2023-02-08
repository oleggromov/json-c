#ifndef MURMUR3_32_H
#define MURMUR3_32_H

#include <stdlib.h> // uint32_t
#include <string.h> // memcpy

uint32_t murmur3_32(const uint8_t* key, size_t len, uint32_t seed);

#endif
