#include "mesh.hpp"
#include "block.hpp"
#include "config.hpp"
#include "logger.hpp"

BlockModel blockModelDefault(int texCoordX, int texCoordY) {
    BlockModel model;
    glm::ivec3 coords = {texCoordX, texCoordY, 0};
    for (int i = 0; i < DIRECTION_COUNT; ++i) {
        model.sides[i] = coords;
    }
    return model;
}

BlockModel blockModelTable(
        int sideATexCoordX,
        int sideATexCoordY,
        int sideBTexCoordX,
        int sideBTexCoordY,
        int topTexCoordX,
        int topTexCoordY,
        int bottomTexCoordX,
        int bottomTexCoordY
) {
    BlockModel model;
    glm::ivec3 sideA = {sideATexCoordX, sideATexCoordY, 0};
    glm::ivec3 sideB = {sideBTexCoordX, sideBTexCoordY, 0};

    model.sides[DIRECTION_NORTH] = sideA;
    model.sides[DIRECTION_SOUTH] = sideA;
    model.sides[DIRECTION_EAST] = sideB;
    model.sides[DIRECTION_WEST] = sideB;

    model.sides[DIRECTION_UP] = (glm::ivec3){topTexCoordX, topTexCoordY, 0};
    model.sides[DIRECTION_DOWN] = (glm::ivec3){bottomTexCoordX, bottomTexCoordY, 0};

    return model;
}

BlockModel blockModelGrass(
        int sideTexCoordX,
        int sideTexCoordY,
        int topTexCoordX,
        int topTexCoordY,
        int bottomTexCoordX,
        int bottomTexCoordY
) {
    return blockModelTable(sideTexCoordX, sideTexCoordY, sideTexCoordX,
            sideTexCoordY, topTexCoordX, topTexCoordY, bottomTexCoordX, bottomTexCoordY);
}

BlockModel blockModelLog(
        int sideTexCoordX,
        int sideTexCoordY,
        int topTexCoordX,
        int topTexCoordY
) {
    return blockModelGrass(sideTexCoordX, sideTexCoordY, topTexCoordX,
            topTexCoordY, topTexCoordX, topTexCoordY);
}


BlockModel blockModelNone() {
    return blockModelDefault(0, 14);
}

BlockProperties blocks[BLOCK_COUNT];

static void regBlock(Block block, const char* name, Solidness solid, Passability passability, BlockModel model) {
    ASSERT(name);

    BlockProperties props = {
        .name = name,
        .solidness = solid,
        .model = model,
        .passability = passability
    };

    blocks[block] = props;
    blocks[block].mesh = blockMesh(block);
}

void registerBlocks() {
    regBlock(BLOCK_AIR, "air", TRANSPARENT, PASSABLE, blockModelNone());
    regBlock(BLOCK_BARRIER, "barrier", SOLID, IMPASSABLE, blockModelNone());

    regBlock(BLOCK_STONE, "stone", SOLID, IMPASSABLE, blockModelDefault(1, 0));
    regBlock(BLOCK_DIRT, "dirt", SOLID, IMPASSABLE, blockModelDefault(2, 0));
    regBlock(BLOCK_PLANKS, "planks", SOLID, IMPASSABLE, blockModelDefault(4, 0));
    regBlock(BLOCK_COBBLESTONE, "cobblestone", SOLID, IMPASSABLE, blockModelDefault(0, 1));
    regBlock(BLOCK_BEDROCK, "bedrock", SOLID, IMPASSABLE, blockModelDefault(1, 1));
    regBlock(BLOCK_DIAMOND_ORE, "diamond_ore", SOLID, IMPASSABLE, blockModelDefault(2, 3));
    regBlock(BLOCK_OBSIDIAN, "obsidian", SOLID, IMPASSABLE, blockModelDefault(5, 2));
    regBlock(BLOCK_GRASS, "grass", SOLID, IMPASSABLE, blockModelGrass(3, 0, 0, 0, 2, 0));
    regBlock(BLOCK_SAND, "sand", SOLID, IMPASSABLE, blockModelDefault(2, 1));
    regBlock(BLOCK_GLASS, "glass", TRANSPARENT, IMPASSABLE, blockModelDefault(1, 3));

#if FAST_LEAVES
    regBlock(BLOCK_LEAVES, "leaves", SOLID, IMPASSABLE, blockModelDefault(5, 3));
#else
    regBlock(BLOCK_LEAVES, "leaves", TRANSPARENT, IMPASSABLE, blockModelDefault(4, 3));
#endif

    regBlock(BLOCK_LOG, "log", SOLID, IMPASSABLE, blockModelLog(4, 1, 5, 1));
    regBlock(BLOCK_CRAFTING_TABLE, "crafting_table", SOLID, IMPASSABLE, blockModelTable(11, 3, 12, 3, 11, 2, 4, 0));
    regBlock(BLOCK_WATER, "water", TRANSLUCENT, PASSABLE, blockModelDefault(13, 12));
    regBlock(BLOCK_SNOW, "snow", SOLID, IMPASSABLE, blockModelDefault(2, 4));
    regBlock(BLOCK_ICE, "ice", TRANSLUCENT, IMPASSABLE, blockModelDefault(3, 4));
    regBlock(BLOCK_CACTUS, "cactus", TRANSPARENT, IMPASSABLE, blockModelGrass(6, 4, 5, 4, 7, 4));
    regBlock(BLOCK_LAVA, "lava", TRANSPARENT, PASSABLE, blockModelDefault(13, 14));
    regBlock(BLOCK_SNOWY_GRASS, "snowy_grass", SOLID, IMPASSABLE, blockModelGrass(4, 4, 2, 4, 2, 0));

    regBlock(BLOCK_COBWEB, "cobweb", CROSS, PASSABLE, blockModelDefault(11, 0));
    regBlock(BLOCK_ROSE, "rose", CROSS, PASSABLE, blockModelDefault(12, 0));
    regBlock(BLOCK_DANDELION, "dandelion", CROSS, PASSABLE, blockModelDefault(13, 0));
}

