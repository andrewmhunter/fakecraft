#include "graphics/mesh.hpp"
#include "block.hpp"
#include "engine/config.hpp"
#include "engine/logger.hpp"

BlockModel blockModelDefault(int texCoordX, int texCoordY) {
    BlockModel model;
    glm::ivec3 coords = {texCoordX, texCoordY, 0};
    for (int i = 0; i < directionCount; ++i) {
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

    model.sides[Direction::north] = sideA;
    model.sides[Direction::south] = sideA;
    model.sides[Direction::east] = sideB;
    model.sides[Direction::west] = sideB;

    model.sides[Direction::up] = (glm::ivec3){topTexCoordX, topTexCoordY, 0};
    model.sides[Direction::down] = (glm::ivec3){bottomTexCoordX, bottomTexCoordY, 0};

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
    regBlock(Block::air, "air", Solidness::transparent, Passability::passable, blockModelNone());
    regBlock(Block::barrier, "barrier", Solidness::solid, Passability::impassable, blockModelNone());

    regBlock(Block::stone, "stone", Solidness::solid, Passability::impassable, blockModelDefault(1, 0));
    regBlock(Block::dirt, "dirt", Solidness::solid, Passability::impassable, blockModelDefault(2, 0));
    regBlock(Block::planks, "planks", Solidness::solid, Passability::impassable, blockModelDefault(4, 0));
    regBlock(Block::cobblestone, "cobblestone", Solidness::solid, Passability::impassable, blockModelDefault(0, 1));
    regBlock(Block::bedrock, "bedrock", Solidness::solid, Passability::impassable, blockModelDefault(1, 1));
    regBlock(Block::diamondOre, "diamond_ore", Solidness::solid, Passability::impassable, blockModelDefault(2, 3));
    regBlock(Block::obsidian, "obsidian", Solidness::solid, Passability::impassable, blockModelDefault(5, 2));
    regBlock(Block::grass, "grass", Solidness::solid, Passability::impassable, blockModelGrass(3, 0, 0, 0, 2, 0));
    regBlock(Block::sand, "sand", Solidness::solid, Passability::impassable, blockModelDefault(2, 1));
    regBlock(Block::glass, "glass", Solidness::transparent, Passability::impassable, blockModelDefault(1, 3));

    if (Config::settings->graphics.fastLeaves) {
        regBlock(Block::leaves, "leaves", Solidness::solid, Passability::impassable, blockModelDefault(5, 3));
    } else {
        regBlock(Block::leaves, "leaves", Solidness::transparent, Passability::impassable, blockModelDefault(4, 3));
    }

    regBlock(Block::log, "log", Solidness::solid, Passability::impassable, blockModelLog(4, 1, 5, 1));
    regBlock(Block::craftingTable, "crafting_table", Solidness::solid, Passability::impassable, blockModelTable(11, 3, 12, 3, 11, 2, 4, 0));
    regBlock(Block::water, "water", Solidness::translucent, Passability::passable, blockModelDefault(13, 12));
    regBlock(Block::snow, "snow", Solidness::solid, Passability::impassable, blockModelDefault(2, 4));
    regBlock(Block::ice, "ice", Solidness::translucent, Passability::impassable, blockModelDefault(3, 4));
    regBlock(Block::cactus, "cactus", Solidness::cactus, Passability::impassable, blockModelGrass(6, 4, 5, 4, 7, 4));
    regBlock(Block::lava, "lava", Solidness::transparent, Passability::passable, blockModelDefault(13, 14));
    regBlock(Block::snowyGrass, "snowy_grass", Solidness::solid, Passability::impassable, blockModelGrass(4, 4, 2, 4, 2, 0));

    regBlock(Block::cobweb, "cobweb", Solidness::cross, Passability::passable, blockModelDefault(11, 0));
    regBlock(Block::rose, "rose", Solidness::cross, Passability::passable, blockModelDefault(12, 0));
    regBlock(Block::dandelion, "dandelion", Solidness::cross, Passability::passable, blockModelDefault(13, 0));
}

void unregisterBlocks() {
    for (int i = 0; i < blockCount; ++i) {
        blocks[i] = std::nullopt;
    }
}
