#ifndef BLOCK_H
#define BLOCK_H

#include <m-lib/m-core.h>
#include <stdbool.h>
#include <raylib.h>

typedef enum : char {
    BLOCK_AIR,
    BLOCK_STONE,
    BLOCK_DIRT,
    BLOCK_PLANKS,
    BLOCK_COBBLESTONE,
    BLOCK_BEDROCK,
    BLOCK_DIAMOND_ORE,
    BLOCK_OBSIDIAN,
    BLOCK_GLASS,
    BLOCK_GRASS,
    BLOCK_SAND,
    BLOCK_LEAVES,
    BLOCK_LOG,
    BLOCK_BLANK,
    BLOCK_COUNT,
} Block;

typedef enum {
    SOLID,
    TRANSPARENT,
} Solidness;

typedef struct {
    const char* name;
    Solidness solidness;
    int texCoordX;
    int texCoordY;
    Mesh mesh;
} BlockProperties;

extern BlockProperties blocks[BLOCK_COUNT];

#endif

