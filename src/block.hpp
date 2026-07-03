#ifndef BLOCK_HPP
#define BLOCK_HPP

#include <stdbool.h>
#include <optional>
#include "direction.hpp"
#include "graphics.hpp"

// Barrier must be 0 so the chunk mesher
// detects blocks outside the loaded chunks as
// non-transparent so the sides of the world are
// culled.

// Block IDs
enum class Block : char {
    barrier,

    air,
    stone,
    dirt,
    planks,
    cobblestone,
    bedrock,
    diamondOre,
    obsidian,
    glass,
    grass,
    sand,
    leaves,
    log,
    craftingTable,
    water,
    snow,
    ice,
    cactus,
    lava,
    snowyGrass,
    cobweb,
    rose,
    dandelion,

    // must be last
    blockCount,
};

constexpr int blockCount = static_cast<int>(Block::blockCount);



typedef enum {
    SOLID,
    TRANSPARENT,
    TRANSLUCENT,
    CROSS,
} Solidness;

typedef enum {
    PASSABLE,
    IMPASSABLE,
} Passability;

typedef struct {
    glm::ivec3 sides[DIRECTION_COUNT];
} BlockModel;

typedef struct {
    const char* name;
    Solidness solidness;
    GPUMesh mesh;
    BlockModel model;
    Passability passability;
} BlockProperties;


extern std::optional<BlockProperties> blocks[blockCount];

static inline const BlockProperties& getBlock(Block block) {
    return *blocks[static_cast<int>(block)];
}

void registerBlocks();
void unregisterBlocks();

#endif

