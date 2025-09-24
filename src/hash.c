#include <stdint.h>
#include <stdlib.h>
#include "hash.h"

Hash hashString(Hash hash, const char* string) {
    int stringLength = strlen(string);
    hash = hashBytes(hash, &stringLength, sizeof(int));
    return hashBytes(hash, string, stringLength);
}

void setInit(Set* set) {
    set->entries = NULL;
    set->capacity = 0;
    set->count = 0;
}

void setUnload(Set* set) {
    free(set->entries);
}

void setUnloadAll(Set* set) {
    setForEach(set, free);
    setUnload(set);
}

void setForEach(Set* set, void (*fn)(void* value)) {
    HashEntry* entry = NULL;
    while (setIterate(set, &entry)) {
        fn(entry->contents);
    }
}

void setExtend(Set* set) {
    Set newSet = {0};
    newSet.capacity = set->capacity < 4 ? 8 : set->capacity * 2;
    newSet.entries = calloc(newSet.capacity, sizeof(HashEntry));

    for (size_t i = 0; i < set->capacity; ++i) {
        HashEntry* oldEntry = &set->entries[i];
        if (oldEntry->type != ENTRY_FULL) {
            continue;
        }

        size_t index = oldEntry->hash;
        for (;;) {
            index %= newSet.capacity;
            HashEntry* newEntry = &newSet.entries[index];
            index++;

            if (newEntry->type == ENTRY_EMPTY) {
                newEntry->type = ENTRY_FULL;
                newEntry->hash = oldEntry->hash;
                newEntry->contents = oldEntry->contents;
                break;
            }
        }

        newSet.count++;
    }

    setUnload(set);
    *set = newSet;
}

bool setIterate(Set* set, HashEntry** entry) {
    if (*entry == NULL) {
        *entry = set->entries - 1;
    }

    for (;;) {
        *entry += 1;

        if (*entry >= set->entries + set->capacity) {
            return false;
        }

        if ((*entry)->type == ENTRY_FULL) {
            return true;
        }
    }
}

