#include "block.h"
#include <raymath.h>

BlockProperties blocks[BLOCK_COUNT] = {
    [BLOCK_AIR]         = {"air", TRANSPARENT, 0, 0},
    [BLOCK_STONE]       = {"stone", SOLID, 1, 0},
    [BLOCK_DIRT]        = {"dirt", SOLID, 2, 0},
    [BLOCK_PLANKS]      = {"planks", SOLID, 4, 0},
    [BLOCK_COBBLESTONE] = {"cobblestone", SOLID, 0, 1},
    [BLOCK_BEDROCK]     = {"bedrock", SOLID, 1, 1},
    [BLOCK_DIAMOND_ORE] = {"diamond_ore", SOLID, 2, 3},
    [BLOCK_OBSIDIAN]    = {"obsidian", SOLID, 5, 2},
    [BLOCK_GRASS]       = {"grass", SOLID, 0, 0},
    //[BLOCK_GRASS]       = {"grass", SOLID, 7, 11},
    [BLOCK_SAND]        = {"sand", SOLID, 2, 1},
    [BLOCK_GLASS]       = {"glass", TRANSPARENT, 1, 3},
    [BLOCK_LEAVES]      = {"leaves", TRANSPARENT, 4, 3},
    [BLOCK_LOG]         = {"log", SOLID, 4, 1},
    //[BLOCK_GLASS] = {"glass", SOLID, 1, 3},
    [BLOCK_BLANK]       = {"blank", SOLID, 0, 14},
};

