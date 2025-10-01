#ifndef HASH_H
#define HASH_H

#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#define FNV_PRIME 0x01000193
#define FNV_OFFSET 0x811c9dc5

#define SET_RATIO_NUMERATOR 1
#define SET_RATIO_DENOMINATOR 2

typedef uint32_t Hash;

static inline Hash hashBytes(Hash hash, const void* data, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        hash ^= ((uint8_t*)data)[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

static inline Hash hashInt(Hash hash, int value) {
    return hashBytes(hash, &value, sizeof(value));
}

Hash hashString(Hash hash, const char* string);

typedef Hash (*HashFn)(Hash hash, const void* data);
typedef bool (*EqualsFn)(const void* left, const void* right);

typedef enum {
    ENTRY_EMPTY = 0,
    ENTRY_FULL,
    ENTRY_GRAVE,
} HashEntryType;

typedef struct {
    HashEntryType type;
    Hash hash;
    void* contents;
} HashEntry;

typedef struct Set {
    HashEntry* entries;
    size_t count;
    size_t capacity;
} Set;

typedef struct {
    HashFn hashFn;
    EqualsFn equalsFn;
} SetVTable;


void setInit(Set* set);
void setUnload(Set* set);
void setUnloadAll(Set* set);
void setExtend(Set* set);
bool setIterate(Set* set, HashEntry** entry);
void setForEach(Set* set, void (*fn)(void* value));

static inline HashEntry* setGetEntry(const Set* set, SetVTable vtable, const void* key, Hash hash) {
    if (set->capacity == 0) {
        return NULL;
    }

    size_t index = hash;

    HashEntry* firstEmpty = NULL;

    for (;;) {
        index %= set->capacity;

        HashEntry* entry = &set->entries[index];

        if (entry->type == ENTRY_EMPTY) {
            return firstEmpty == NULL ? entry : firstEmpty;
        }

        if (entry->type == ENTRY_FULL && hash == entry->hash && vtable.equalsFn(entry->contents, key)) {
            return entry;
        }

        if (entry->type == ENTRY_GRAVE && firstEmpty == NULL) {
            firstEmpty = entry;
        }

        index++;
    }
}

static inline void* setGet(const Set* set, SetVTable vtable, const void* key) {
    Hash hash = vtable.hashFn(FNV_OFFSET, key);
    HashEntry* entry = setGetEntry(set, vtable, key, hash);
    if (entry != NULL && entry->type == ENTRY_FULL) {
        return entry->contents;
    }
    return NULL;
}

static inline bool setHas(const Set* set, SetVTable vtable, const void* key) {
    return setGet(set, vtable, key) != NULL;
}

static inline void* setInsert(Set* set, SetVTable vtable, void* keyValue) {
    Hash hash = vtable.hashFn(FNV_OFFSET, keyValue);

    if (set->entries == NULL) {
        setExtend(set);
    }

    void* oldValue = NULL;

    HashEntry* entry = setGetEntry(set, vtable, keyValue, hash);

    if (entry->type == ENTRY_EMPTY) {
        set->count += 1;
    }

    if (entry->type == ENTRY_FULL) {
        oldValue = entry->contents;
    }

    entry->type = ENTRY_FULL;
    entry->hash = hash;
    entry->contents = keyValue;

    if (SET_RATIO_DENOMINATOR * set->count > SET_RATIO_NUMERATOR * set->capacity) {
        setExtend(set);
    }

    return oldValue;
}

static inline void* setRemove(Set* set, SetVTable vtable, const void* key) {
    Hash hash = vtable.hashFn(FNV_OFFSET, key);
    HashEntry* entry = setGetEntry(set, vtable, key, hash);
    if (entry == NULL || entry->type != ENTRY_FULL) {
        return NULL;
    }

    entry->type = ENTRY_GRAVE;
    return entry->contents;
}

#endif
