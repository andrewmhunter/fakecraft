#include <raylib.h>
#include "chunk.h"
#include "world.h"
#include "worldgen.h"
#include "logger.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"
#undef STB_PERLIN_IMPLEMENTATION

void placeDungeon(World* world, Point worldPoint) {
    ASSERT(world);

    worldPlaceBox(world, worldPoint, point(10, 6, 10), BLOCK_COBBLESTONE);
    worldPlaceBox(world, pointAddValue(worldPoint, 1, 1, 1), point(8, 4, 8), BLOCK_AIR);
}

static void placeCactus(World* world, Point worldPoint) {
    int cactusHeight = randomRange(1, 4);
    worldPoint = pointAddY(worldPoint, 1);
    if (worldGetBlock(world, worldPoint) != BLOCK_AIR) {
        return;
    }

    for (int i = 0; i < cactusHeight; ++i) {
        worldSetBlock(world, pointAddY(worldPoint, i), BLOCK_CACTUS);
    }
}

void placeTree(World* world, Point worldPoint) {
    ASSERT(world);

    Block surfaceBlock = worldGetBlock(world, worldPoint);

    if (surfaceBlock == BLOCK_SAND) {
        placeCactus(world, worldPoint);
        return;
    }

    if (surfaceBlock != BLOCK_GRASS) {
        return;
    }

    static const Point corners[4] = {
        {1, 0, 1},
        {-1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
    };

    worldSetBlock(world, worldPoint, BLOCK_DIRT);
    worldPoint = pointAddY(worldPoint, 1);

    int height = randomRange(5, 7);
    worldPlaceBox(world, worldPoint, point(1, height, 1), BLOCK_LOG);

    //worldTryPlaceBox(world, pointAddValue(worldPoint, -2, 2, -2), point(5, 2, 5), BLOCK_LEAVES);

    worldTryPlaceBox(world, pointAddValue(worldPoint, -2, height - 3, -1), point(5, 2, 3), BLOCK_LEAVES);
    worldTryPlaceBox(world, pointAddValue(worldPoint, -1, height - 3, -2), point(3, 2, 5), BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        Point scaledCorner = pointScale(corners[i], 2);
        for (int j = height - 3; j < height - 1; ++j) {
            if (!randomChance(1, 2)) {
                continue;
            }
            worldTryPlaceBlock(world, pointAdd(worldPoint, pointAddY(scaledCorner, j)), BLOCK_LEAVES);
        }
    }

    worldTryPlaceBox(world, pointAddValue(worldPoint, -1, height - 1, 0), point(3, 2, 1), BLOCK_LEAVES);
    worldTryPlaceBox(world, pointAddValue(worldPoint, 0, height - 1, -1), point(1, 2, 3), BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        if (!randomChance(1, 2)) {
            continue;
        }

        worldTryPlaceBlock(world, pointAdd(worldPoint, pointAddY(corners[i], height - 1)), BLOCK_LEAVES);
    }
}

void generateTerrain(Chunk* chunk) {
    ASSERT(chunk);

    World* world = chunk->world;
    Point coords = chunk->coords;

    ITERATE_CHUNK(x, y, z) {
        chunk->blocks[x][y][z] = BLOCK_AIR;
    }

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            int surface = SURFACE_OFFSET;

#ifndef SUPERFLAT
            float biomeScale = 0.001f;

            float wx = x + CHUNK_WIDTH * coords.x;
            float wz = z + CHUNK_WIDTH * coords.z;

            float biome = stb_perlin_fbm_noise3(wx * biomeScale, 2.f, wz * biomeScale, 2.f, 0.5f, 10) + 0.5f;
            biome *= 255.f;

            float scale = 0.001f;
            float stretch = 128.f;
            int octaves = 12;
            float noise = stb_perlin_fbm_noise3(wx * scale, wz * scale, 1.f, 2.f, 0.5f, octaves);
            surface = noise * stretch + SURFACE_OFFSET;
#endif

            surface = clampInt(surface, 1, CHUNK_HEIGHT - 1);

            chunk->surfaceHeight[x][z] = surface;

            chunk->blocks[x][0][z] = BLOCK_BEDROCK;

            Block topLayerBlock = BLOCK_DIRT;
            Block surfaceBlock = BLOCK_GRASS;

            if (biome < 64.0) {
                surfaceBlock = BLOCK_SNOWY_GRASS;
            }

            if (surface < OCEAN_LEVEL + 3 || biome > 192.0) {
                topLayerBlock = BLOCK_SAND;
                surfaceBlock = BLOCK_SAND;
            }

            for (int y = 1; y <= surface; ++y) {
                float caveThreshold = -0.5f;
                float caveScale = 0.025;
                float caveNoise = stb_perlin_fbm_noise3(y * caveScale, wz * caveScale, wx * caveScale, 2.f, 0.5f, 4);

                if (caveNoise < caveThreshold) {
                    continue;
                }

                Block block = topLayerBlock;
                if (y < surface - DIRT_LAYER) {
                    block = BLOCK_STONE;
                }

                if (y == surface) {
                    block = surfaceBlock;
                }

                chunk->blocks[x][y][z] = block;
            }

            for (int y = 11; y >= 0; --y) {
                if (chunk->blocks[x][y][z] != BLOCK_AIR) {
                    continue;
                }
                chunk->blocks[x][y][z] = BLOCK_LAVA;
            }

            if (surface < OCEAN_LEVEL) {
                for (int y = OCEAN_LEVEL; y > 0; --y) {
                    if (chunk->blocks[x][y][z] != BLOCK_AIR) {
                        break;
                    }

                    Block waterBlock = BLOCK_WATER;
                    if (y == OCEAN_LEVEL && biome < 64.f) {
                        waterBlock = BLOCK_ICE;
                    }
                    chunk->blocks[x][y][z] = waterBlock;
                }
            }


            //chunk->blocks[x][surface][z] = surfaceBlock;
        }
    }

    chunk->dirty = true;
}

void placeFeatures(Chunk* chunk) {
#ifndef SUPERFLAT
    int treeCount = randomInt(5);
    for (int i = 0; i < treeCount; ++i) {
        Point point = {randomInt(CHUNK_WIDTH - 4) + 2, 0, randomInt(CHUNK_WIDTH - 4) + 2};
        point.y = chunk->surfaceHeight[point.x][point.z];

        placeTree(chunk->world, localToWorld(chunk->coords, point));
    }

    if (randomChance(1, 20)) {
        placeDungeon(chunk->world, localToWorld(chunk->coords, point(randomInt(CHUNK_WIDTH - 10), 10, randomInt(CHUNK_WIDTH - 10))));
    }
#endif
}

