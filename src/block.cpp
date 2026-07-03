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

std::optional<BlockProperties> blocks[blockCount];

static void regBlock(Block block, const char* name, Solidness solid, Passability passability, BlockModel model) {
    Logger::assertion(name);

    BlockProperties props = {
        .name = name,
        .solidness = solid,
        .mesh = {GL_TRIANGLES},
        .model = model,
        .passability = passability,
    };

    blocks[static_cast<int>(block)] = std::move(props);
    blocks[static_cast<int>(block)]->mesh = blockMesh(block);
}

void registerBlocks() {
    regBlock(Block::air, "air", TRANSPARENT, PASSABLE, blockModelNone());
    regBlock(Block::barrier, "barrier", SOLID, IMPASSABLE, blockModelNone());

    regBlock(Block::stone, "stone", SOLID, IMPASSABLE, blockModelDefault(1, 0));
    regBlock(Block::dirt, "dirt", SOLID, IMPASSABLE, blockModelDefault(2, 0));
    regBlock(Block::planks, "planks", SOLID, IMPASSABLE, blockModelDefault(4, 0));
    regBlock(Block::cobblestone, "cobblestone", SOLID, IMPASSABLE, blockModelDefault(0, 1));
    regBlock(Block::bedrock, "bedrock", SOLID, IMPASSABLE, blockModelDefault(1, 1));
    regBlock(Block::diamondOre, "diamond_ore", SOLID, IMPASSABLE, blockModelDefault(2, 3));
    regBlock(Block::obsidian, "obsidian", SOLID, IMPASSABLE, blockModelDefault(5, 2));
    regBlock(Block::grass, "grass", SOLID, IMPASSABLE, blockModelGrass(3, 0, 0, 0, 2, 0));
    regBlock(Block::sand, "sand", SOLID, IMPASSABLE, blockModelDefault(2, 1));
    regBlock(Block::glass, "glass", TRANSPARENT, IMPASSABLE, blockModelDefault(1, 3));

    if (Config::settings->graphics.fastLeaves) {
        regBlock(Block::leaves, "leaves", SOLID, IMPASSABLE, blockModelDefault(5, 3));
    } else {
        regBlock(Block::leaves, "leaves", TRANSPARENT, IMPASSABLE, blockModelDefault(4, 3));
    }

    regBlock(Block::log, "log", SOLID, IMPASSABLE, blockModelLog(4, 1, 5, 1));
    regBlock(Block::craftingTable, "crafting_table", SOLID, IMPASSABLE, blockModelTable(11, 3, 12, 3, 11, 2, 4, 0));
    regBlock(Block::water, "water", TRANSLUCENT, PASSABLE, blockModelDefault(13, 12));
    regBlock(Block::snow, "snow", SOLID, IMPASSABLE, blockModelDefault(2, 4));
    regBlock(Block::ice, "ice", TRANSLUCENT, IMPASSABLE, blockModelDefault(3, 4));
    regBlock(Block::cactus, "cactus", TRANSPARENT, IMPASSABLE, blockModelGrass(6, 4, 5, 4, 7, 4));
    regBlock(Block::lava, "lava", TRANSPARENT, PASSABLE, blockModelDefault(13, 14));
    regBlock(Block::snowyGrass, "snowy_grass", SOLID, IMPASSABLE, blockModelGrass(4, 4, 2, 4, 2, 0));

    regBlock(Block::cobweb, "cobweb", CROSS, PASSABLE, blockModelDefault(11, 0));
    regBlock(Block::rose, "rose", CROSS, PASSABLE, blockModelDefault(12, 0));
    regBlock(Block::dandelion, "dandelion", CROSS, PASSABLE, blockModelDefault(13, 0));
}

void unregisterBlocks() {
    for (int i = 0; i < blockCount; ++i) {
        blocks[i] = std::nullopt;
    }
}
