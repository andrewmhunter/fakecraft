#include <stb_perlin.h>
#include "block.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "util/hash.hpp"
#include "worldgen.hpp"
#include "engine/logger.hpp"

#define DIRT_LAYER 2
#define SURFACE_OFFSET 65
#define OCEAN_LEVEL 60

Hash featureSeed(Hash seed, FeatureId feature) {
    return hashChar(seed, feature);
}

int seededNumber(Hash chunkSeed, FeatureId feature, int min, int max) {
    unsigned int number = featureSeed(chunkSeed, feature);
    return (number % (max - min)) + min;
}

void placeDungeon(World* world, glm::ivec3 worldPoint) {
    Logger::assertion(world);

    world->placeBox(worldPoint, {10, 6, 10}, Block::cobblestone);
    world->placeBox(worldPoint + 1, {8, 4, 8}, Block::air);
}

static void placeCactus(World* world, glm::ivec3 worldPoint) {
    int cactusHeight = randomRange(1, 4);
    worldPoint += glm::ivec3{0, 1, 0};
    if (world->getBlock(worldPoint) != Block::air) {
        return;
    }

    for (int i = 0; i < cactusHeight; ++i) {
        world->setBlock(worldPoint + glm::ivec3{0, i, 0}, Block::cactus);
    }
}

void placeTree(World* world, glm::ivec3 worldPoint) {
    Logger::assertion(world);
    Logger::debug("Placing Tree");

    Block surfaceBlock = world->getBlock(worldPoint);

    if (surfaceBlock == Block::sand) {
        placeCactus(world, worldPoint);
        return;
    }

    if (surfaceBlock != Block::grass) {
        return;
    }

    static const glm::ivec3 corners[4] = {
        {1, 0, 1},
        {-1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
    };

    world->setBlock(worldPoint, Block::dirt);
    worldPoint += glm::ivec3{0, 1, 0};

    int height = randomRange(5, 7);
    world->placeBox(worldPoint, {1, height, 1}, Block::log);

    //worldTryPlaceBox(world, pointAddValue(worldPoint, -2, 2, -2), point(5, 2, 5), Block::leaves);

    world->tryPlaceBox(worldPoint + glm::ivec3{-2, height - 3, -1}, {5, 2, 3}, Block::leaves);
    world->tryPlaceBox(worldPoint + glm::ivec3{-1, height - 3, -2}, {3, 2, 5}, Block::leaves);

    for (int i = 0; i < 4; ++i) {
        glm::ivec3 scaledCorner = corners[i] * 2;
        for (int j = height - 3; j < height - 1; ++j) {
            if (!randomChance(1, 2)) {
                continue;
            }
            world->tryPlaceBlock(worldPoint + scaledCorner + glm::ivec3{0, j, 0}, Block::leaves);
        }
    }

    world->tryPlaceBox(worldPoint + glm::ivec3{-1, height - 1, 0}, {3, 2, 1}, Block::leaves);
    world->tryPlaceBox(worldPoint + glm::ivec3{0, height - 1, -1}, {1, 2, 3}, Block::leaves);

    for (int i = 0; i < 4; ++i) {
        if (!randomChance(1, 2)) {
            continue;
        }

        world->tryPlaceBlock(worldPoint + corners[i] + glm::ivec3{0, height - 1, 0}, Block::leaves);
    }
}

void generateTerrain(Chunk* chunk) {
    Logger::assertion(chunk);

    //World* world = chunk->world;
    glm::ivec3 coords = chunk->coords;

    ITERATE_CHUNK(x, y, z) {
        chunk->blocks[x][y][z] = Block::air;
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

            if (!Config::settings->world.superflat) {
                float scale = 0.004f;
                float stretch = 32.f;
                int octaves = 15;
                float noise = stb_perlin_fbm_noise3(wx * scale, wz * scale, 1.f, 2.f, 0.5f, octaves);
                surface = noise * stretch + SURFACE_OFFSET;
            }

            surface = clampInt(surface, 1, CHUNK_HEIGHT - 1);

            chunk->surfaceHeight[x][z] = surface;

            chunk->blocks[x][0][z] = Block::bedrock;

            Block topLayerBlock = Block::dirt;
            Block surfaceBlock = Block::grass;

            if (biome < 64.0) {
                surfaceBlock = Block::snowyGrass;
            }

            if (surface < OCEAN_LEVEL + 3 || biome > 192.0) {
                topLayerBlock = Block::sand;
                surfaceBlock = Block::sand;
            }

            for (int y = 1; y <= surface; ++y) {
                if (Config::settings->world.generateCaves) {
                    float caveThreshold = -0.5f;
                    float caveScale = 0.05;
                    float caveNoise = stb_perlin_fbm_noise3(y * caveScale, wz * caveScale, wx * caveScale, 2.f, 0.5f, 4);

                    if (caveNoise < caveThreshold) {
                        continue;
                    }
                }

                Block block = topLayerBlock;
                if (y < surface - DIRT_LAYER) {
                    block = Block::stone;
                }

                if (y == surface) {
                    block = surfaceBlock;
                }

                chunk->blocks[x][y][z] = block;
                chunk->light[x][y][z] = (LightValues){0, 0x80};
            }

            for (int y = 11; y >= 0; --y) {
                if (chunk->blocks[x][y][z] != Block::air) {
                    continue;
                }
                chunk->blocks[x][y][z] = Block::lava;
            }

            if (surface < OCEAN_LEVEL) {
                for (int y = OCEAN_LEVEL; y > 0; --y) {
                    if (chunk->blocks[x][y][z] != Block::air) {
                        break;
                    }

                    Block waterBlock = Block::water;
                    if (y == OCEAN_LEVEL && biome < 64.f) {
                        waterBlock = Block::ice;
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
    if (!Config::settings->world.generateFeatures) {
        return;
    }

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

    Block flowerType = seededNumber(flowerSeed, FEATURE_FLAG, 0, 2) ? Block::rose : Block::dandelion;
    Hash flowerIndexSeed = featureSeed(flowerSeed, FEATURE_INDEX);
    for (int i = 0; i < flowerCount; ++i) {
        Hash flowerIndex = hashChar(flowerIndexSeed, i);

        int x = seededNumber(flowerIndex, FEATURE_X, 0, CHUNK_WIDTH);
        int z = seededNumber(flowerIndex, FEATURE_Z, 0, CHUNK_WIDTH);
        int y = chunk->surfaceHeight[x][z];

        if (chunk->getBlock(glm::ivec3{x, y, z}) == Block::grass) {
            chunk->tryPlaceBlock(x, y + 1, z, flowerType);
        }
    }
}

