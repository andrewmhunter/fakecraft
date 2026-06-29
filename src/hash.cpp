#include <stdint.h>
#include <stdlib.h>
#include "hash.hpp"

Hash hashString(Hash hash, const char* string) {
    int stringLength = strlen(string);
    hash = hashBytes(hash, &stringLength, sizeof(int));
    return hashBytes(hash, string, stringLength);
}
