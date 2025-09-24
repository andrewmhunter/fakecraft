#include <raylib.h>
#include "chunk.h"
#include "world.h"
#include "worldgen.h"
#include "logger.h"

void placeDungeon(World* world, Point worldPoint) {
    ASSERT(world);

    worldPlaceBox(world, worldPoint, point(10, 6, 10), BLOCK_COBBLESTONE);
    worldPlaceBox(world, pointAddValue(worldPoint, 1, 1, 1), point(8, 4, 8), BLOCK_AIR);
}

void placeTree(World* world, Point worldPoint) {
    ASSERT(world);

    if (worldGetBlock(world, worldPoint) != BLOCK_GRASS) {
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

Image noise(const World* world, Point coords, float scale) {
    return GenImagePerlinNoise(CHUNK_WIDTH, CHUNK_WIDTH, 
            CHUNK_WIDTH * coords.x + world->seed, CHUNK_WIDTH * coords.z, scale);
}

void generateTerrain(Chunk* chunk) {
    ASSERT(chunk);

    World* world = chunk->world;
    Point coords = chunk->coords;

    ITERATE_CHUNK(x, y, z) {
        chunk->blocks[x][y][z] = BLOCK_AIR;
    }

#ifndef SUPERFLAT
    Image noise4 = noise(world, coords, 0.02f);
    Image noise3 = noise(world, coords, 0.05f);
    Image noise2 = noise(world, coords, 0.06125f);
    Image noise1 = noise(world, coords, 0.125f);
    Image noise0 = noise(world, coords, 0.25f);
#endif

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            int surface = SURFACE_OFFSET;
#ifndef SUPERFLAT
            float c4 = Clamp(GetImageColor(noise4, x, z).r - 128.f, 0.f, 256.f);

            float c3 = Clamp(GetImageColor(noise3, x, z).r - 170.f, 0.f, 256.f);
            float c2 = GetImageColor(noise2, x, z).r - 128.f;
            float c1 = GetImageColor(noise1, x, z).r - 128.f;
            float c0 = GetImageColor(noise0, x, z).r - 128.f;

            surface += c3 * 1.5f + c2 / 4 + c1 / 8 + c0 / 16 - c4;
#endif
            surface = clampInt(surface, 1, CHUNK_HEIGHT - 1);

            surface = surface >= CHUNK_HEIGHT ? CHUNK_HEIGHT - 1 : surface;
            chunk->surfaceHeight[x][z] = surface;

            chunk->blocks[x][0][z] = BLOCK_BEDROCK;

            Block topLayerBlock = BLOCK_DIRT;
            Block surfaceBlock = BLOCK_GRASS;

            if (surface < 50) {
                topLayerBlock = BLOCK_SAND;
                surfaceBlock = BLOCK_SAND;
            }

            /*if (surface > 100) {
                topLayerBlock = BLOCK_STONE;
                surfaceBlock = BLOCK_STONE;
            }

            if (surface > 120) {
                topLayerBlock = BLOCK_SNOW;
                surfaceBlock = BLOCK_SNOW;
            }*/

            for (int y = 1; y < surface; ++y) {
                Block block = topLayerBlock;
                if (y < surface - DIRT_LAYER) {
                    block = BLOCK_STONE;
                }
                chunk->blocks[x][y][z] = block;
            }

            for (int y = surface + 1; y <= OCEAN_LEVEL; ++y) {
                chunk->blocks[x][y][z] = BLOCK_WATER;
            }

            chunk->blocks[x][surface][z] = surfaceBlock;
        }
    }

    chunk->dirty = true;

#ifndef SUPERFLAT
    UnloadImage(noise3);
    UnloadImage(noise4);
    UnloadImage(noise2);
    UnloadImage(noise1);
    UnloadImage(noise0);
#endif
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

