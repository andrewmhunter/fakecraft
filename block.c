#include "mesh.h"
#include "block.h"
#include <raymath.h>

#define FAST_LEAVES 0

BlockModel blockModelDefault(int texCoordX, int texCoordY) {
    BlockModel model;
    Point coords = {texCoordX, texCoordY, 0};
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
    Point sideA = {sideATexCoordX, sideATexCoordY, 0};
    Point sideB = {sideBTexCoordX, sideBTexCoordY, 0};

    model.sides[DIRECTION_NORTH] = sideA;
    model.sides[DIRECTION_SOUTH] = sideA;
    model.sides[DIRECTION_EAST] = sideB;
    model.sides[DIRECTION_WEST] = sideB;

    model.sides[DIRECTION_UP] = (Point){topTexCoordX, topTexCoordY, 0};
    model.sides[DIRECTION_DOWN] = (Point){bottomTexCoordX, bottomTexCoordY, 0};

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

void regBlock(Block block, const char* name, Solidness solid, BlockModel model) {
    assert(name);

    BlockProperties props = {
        .name = name,
        .solidness = solid,
        .model = model,
    };

    props.mesh = cubeMesh(model.sides[0].x, model.sides[0].y);
    
    blocks[block] = props;
}

void registerBlocks() {
    regBlock(BLOCK_AIR, "air", TRANSPARENT, blockModelNone());
    regBlock(BLOCK_BARRIER, "barrier", SOLID, blockModelNone());

    regBlock(BLOCK_STONE, "stone", SOLID, blockModelDefault(1, 0));
    regBlock(BLOCK_DIRT, "dirt", SOLID, blockModelDefault(2, 0));
    regBlock(BLOCK_PLANKS, "planks", SOLID, blockModelDefault(4, 0));
    regBlock(BLOCK_COBBLESTONE, "cobblestone", SOLID, blockModelDefault(0, 1));
    regBlock(BLOCK_BEDROCK, "bedrock", SOLID, blockModelDefault(1, 1));
    regBlock(BLOCK_DIAMOND_ORE, "diamond_ore", SOLID, blockModelDefault(2, 3));
    regBlock(BLOCK_OBSIDIAN, "obsidian", SOLID, blockModelDefault(5, 2));
    regBlock(BLOCK_GRASS, "grass", SOLID, blockModelGrass(3, 0, 0, 0, 2, 0));
    regBlock(BLOCK_SAND, "sand", SOLID, blockModelDefault(2, 1));
    regBlock(BLOCK_GLASS, "glass", TRANSPARENT, blockModelDefault(1, 3));

    if (FAST_LEAVES) {
        regBlock(BLOCK_LEAVES, "leaves", SOLID, blockModelDefault(5, 3));
    } else {
        regBlock(BLOCK_LEAVES, "leaves", TRANSPARENT, blockModelDefault(4, 3));
    }

    regBlock(BLOCK_LOG, "log", SOLID, blockModelLog(4, 1, 5, 1));
    regBlock(BLOCK_CRAFTING_TABLE, "crafting_table", SOLID, blockModelTable(11, 3, 12, 3, 11, 2, 4, 0));
}

