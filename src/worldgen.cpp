#include <stb_perlin.h>
#include "chunk.hpp"
#include "world.hpp"
#include "hash.hpp"
#include "worldgen.hpp"
#include "logger.hpp"

Hash featureSeed(Hash seed, FeatureId feature) {
    return hashChar(seed, feature);
}

int seededNumber(Hash chunkSeed, FeatureId feature, int min, int max) {
    unsigned int number = featureSeed(chunkSeed, feature);
    return (number % (max - min)) + min;
}

void placeDungeon(World* world, glm::ivec3 worldPoint) {
    Logger::assertion(world);

    world->placeBox(worldPoint, {10, 6, 10}, BLOCK_COBBLESTONE);
    world->placeBox(worldPoint + 1, {8, 4, 8}, BLOCK_AIR);
}

static void placeCactus(World* world, glm::ivec3 worldPoint) {
    int cactusHeight = randomRange(1, 4);
    worldPoint += glm::ivec3{0, 1, 0};
    if (world->getBlock(worldPoint) != BLOCK_AIR) {
        return;
    }

    for (int i = 0; i < cactusHeight; ++i) {
        world->setBlock(worldPoint + glm::ivec3{0, i, 0}, BLOCK_CACTUS);
    }
}

void placeTree(World* world, glm::ivec3 worldPoint) {
    Logger::assertion(world);

    Block surfaceBlock = world->getBlock(worldPoint);

    if (surfaceBlock == BLOCK_SAND) {
        placeCactus(world, worldPoint);
        return;
    }

    if (surfaceBlock != BLOCK_GRASS) {
        return;
    }

    static const glm::ivec3 corners[4] = {
        {1, 0, 1},
        {-1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
    };

    world->setBlock(worldPoint, BLOCK_DIRT);
    worldPoint += glm::ivec3{0, 1, 0};

    int height = randomRange(5, 7);
    world->placeBox(worldPoint, {1, height, 1}, BLOCK_LOG);

    //worldTryPlaceBox(world, pointAddValue(worldPoint, -2, 2, -2), point(5, 2, 5), BLOCK_LEAVES);

    world->tryPlaceBox(worldPoint + glm::ivec3{-2, height - 3, -1}, {5, 2, 3}, BLOCK_LEAVES);
    world->tryPlaceBox(worldPoint + glm::ivec3{-1, height - 3, -2}, {3, 2, 5}, BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        glm::ivec3 scaledCorner = corners[i] * 2;
        for (int j = height - 3; j < height - 1; ++j) {
            if (!randomChance(1, 2)) {
                continue;
            }
            world->tryPlaceBlock(worldPoint + scaledCorner + glm::ivec3{0, j, 0}, BLOCK_LEAVES);
        }
    }

    world->tryPlaceBox(worldPoint + glm::ivec3{-1, height - 1, 0}, {3, 2, 1}, BLOCK_LEAVES);
    world->tryPlaceBox(worldPoint + glm::ivec3{0, height - 1, -1}, {1, 2, 3}, BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        if (!randomChance(1, 2)) {
            continue;
        }

        world->tryPlaceBlock(worldPoint + corners[i] + glm::ivec3{0, height - 1, 0}, BLOCK_LEAVES);
    }
}

void generateTerrain(Chunk* chunk) {
    Logger::assertion(chunk);

    //World* world = chunk->world;
    glm::ivec3 coords = chunk->coords;

    ITERATE_CHUNK(x, y, z) {
        chunk->blocks[x][y][z] = BLOCK_AIR;
        chunk->light[x][y][z] = (LightValues){0x0, 0xff};
    }

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            int surface = SURFACE_OFFSET;

            float biomeScale = 0.001f;

            float wx = x + CHUNK_WIDTH * coords.x;
            float wz = z + CHUNK_WIDTH * coords.z;

            float biome = stb_perlin_fbm_noise3(wx * biomeScale, 2.f, wz * biomeScale, 2.f, 0.5f, 10) + 0.5f;
            biome *= 255.f;

#if !SUPERFLAT
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
#if GENERATE_CAVES
                float caveThreshold = -0.5f;
                float caveScale = 0.025;
                float caveNoise = stb_perlin_fbm_noise3(y * caveScale, wz * caveScale, wx * caveScale, 2.f, 0.5f, 4);

                if (caveNoise < caveThreshold) {
                    continue;
                }
#endif

                Block block = topLayerBlock;
                if (y < surface - DIRT_LAYER) {
                    block = BLOCK_STONE;
                }

                if (y == surface) {
                    block = surfaceBlock;
                }

                chunk->blocks[x][y][z] = block;
                chunk->light[x][y][z] = (LightValues){0, 0x80};
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
#if SUPERFLAT
    return;
#endif

    Hash chunkSeed = hashInt(FNV_OFFSET, chunk->world->seed);
    chunkSeed = hashInt(chunkSeed, chunk->coords.x);
    chunkSeed = hashInt(chunkSeed, chunk->coords.z);

    Hash treeSeed = featureSeed(chunkSeed, FEATURE_TREE);
    int treeCount = seededNumber(treeSeed, FEATURE_NUMBER, 0, 7);

    Hash treeIndexSeed = featureSeed(treeSeed, FEATURE_INDEX);
    for (int i = 0; i < treeCount; ++i) {
        Hash treeIndex = hashChar(treeIndexSeed, i);

        int x = seededNumber(treeIndex, FEATURE_X, 2, CHUNK_WIDTH - 4);
        int z = seededNumber(treeIndex, FEATURE_Z, 2, CHUNK_WIDTH - 4);

        glm::ivec3 point = {x, 0, z};
        point.y = chunk->surfaceHeight[point.x][point.z];

        placeTree(chunk->world, localToWorld(chunk->coords, point));
    }

    if (randomChance(1, 20)) {
        placeDungeon(chunk->world, localToWorld(chunk->coords, {randomInt(CHUNK_WIDTH - 10), 10, randomInt(CHUNK_WIDTH - 10)}));
    }

    Hash flowerSeed = featureSeed(chunkSeed, FEATURE_FLOWER_PATCH);
    int flowerCount = seededNumber(flowerSeed, FEATURE_NUMBER, -10, 10);

    Block flowerType = seededNumber(flowerSeed, FEATURE_FLAG, 0, 2) ? BLOCK_ROSE : BLOCK_DANDELION;
    Hash flowerIndexSeed = featureSeed(flowerSeed, FEATURE_INDEX);
    for (int i = 0; i < flowerCount; ++i) {
        Hash flowerIndex = hashChar(flowerIndexSeed, i);

        int x = seededNumber(flowerIndex, FEATURE_X, 0, CHUNK_WIDTH);
        int z = seededNumber(flowerIndex, FEATURE_Z, 0, CHUNK_WIDTH);
        int y = chunk->surfaceHeight[x][z];

        chunk->tryPlaceBlock(x, y + 1, z, flowerType);
    }
}

