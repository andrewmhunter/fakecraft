#ifndef HASH_HPP
#define HASH_HPP

#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define FNV_PRIME 0x01000193
#define FNV_OFFSET 0x811c9dc5

#define SET_RATIO_NUMERATOR 1
#define SET_RATIO_DENOMINATOR 4

typedef uint32_t Hash;

static inline Hash hashBytes(Hash hash, const void* data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        hash ^= ((uint8_t*)data)[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

static inline Hash hashChar(Hash hash, char ch) {
    return hashBytes(hash, &ch, sizeof(char));
}

static inline Hash hashInt(Hash hash, int value) {
    return hashBytes(hash, &value, sizeof(value));
}

Hash hashString(Hash hash, const char* string);

#endif
