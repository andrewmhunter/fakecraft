#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <raylib.h>
#include "point.h"
#include "direction.h"

// Barrier must be 0 so the chunk mesher
// detects blocks outside the loaded chunks as
// non-transparent so the sides of the world are
// culled.

// Block IDs
typedef enum : char {
    BLOCK_BARRIER,

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
    BLOCK_CRAFTING_TABLE,
    BLOCK_WATER,
    BLOCK_SNOW,
    BLOCK_ICE,
    BLOCK_CACTUS,
    BLOCK_LAVA,
    BLOCK_SNOWY_GRASS,
    BLOCK_COBWEB,
    BLOCK_ROSE,
    BLOCK_DANDELION,

    // Must be last
    BLOCK_COUNT,
} Block;

typedef enum {
    SOLID,
    TRANSPARENT,
    TRANSLUCENT,
    CROSS,
} Solidness;

typedef struct {
    Point sides[DIRECTION_COUNT];
} BlockModel;

typedef struct {
    const char* name;
    Solidness solidness;
    Mesh mesh;
    BlockModel model;
} BlockProperties;

extern BlockProperties blocks[BLOCK_COUNT];

void registerBlocks();

#endif

