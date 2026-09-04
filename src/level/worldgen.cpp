#include <bit>
#include <glm/ext/vector_int3.hpp>
#include <stb_perlin.h>
#include "block.hpp"
#include "chunk.hpp"
#include "engine/config.hpp"
#include "util/direction.hpp"
#include "util/types.hpp"
#include "world.hpp"
#include "worldgen.hpp"
#include "engine/logger.hpp"

constexpr int dirtLayer = 3;
constexpr int surfaceOffset = 65;
constexpr int seaLevel = 60;


class WorldGenRNG {
private:
    u32 seed;

public:
    WorldGenRNG(int worldSeed, glm::ivec3 chunkPosition)
        : seed{std::rotl(static_cast<u32>(worldSeed), 2) ^ std::rotl(static_cast<u32>(chunkPosition.x), 1) ^ chunkPosition.z}
    {}

    // https://en.wikipedia.org/wiki/Xorshift
    u32 next() {
        seed ^= seed << 14;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        return seed;
    }

    i32 getInt(i32 max) {
        if (max <= 0) {
            return 0;
        }
        return next() % max;
    }

    i32 getInt(i32 min, i32 max) {
        return getInt(max - min) + min;
    }

    glm::ivec3 getSurfacePosition(Chunk* chunk, int border = 0) {
        i32 x = getInt(border, CHUNK_WIDTH - border);
        i32 z = getInt(border, CHUNK_WIDTH - border);
        i32 y = chunk->surfaceHeight[x][z];

        return glm::ivec3{x, y, z};
    }

    bool chance(int numerator, int demominator) {
        return getInt(demominator) < numerator;
    }
};



constexpr int biomeLookupResolution = 5;

// biomeLookup[temp][humidity]
constexpr Biome biomeLookup[biomeLookupResolution][biomeLookupResolution] = {
    {Biome::tundra, Biome::tundra, Biome::tundra, Biome::taiga, Biome::taiga},
    {Biome::tundra, Biome::tundra, Biome::taiga, Biome::taiga, Biome::taiga},
    {Biome::plains, Biome::plains, Biome::meadow, Biome::forest, Biome::forest},
    {Biome::desert, Biome::plains, Biome::forest, Biome::forest, Biome::jungle},
    {Biome::desert, Biome::desert, Biome::plains, Biome::jungle, Biome::jungle},
};

struct BiomeProperties {
    Block surfaceBlock;
    Block undergroundBlock;

    int beachSize;

    int minTrees;
    int maxTrees;
};



void placeDungeon(Chunk& chunk, glm::ivec3 local) {
    chunk.placeBoxRaw(local, {10, 6, 10}, Block::cobblestone);
    chunk.placeBoxRaw(local + 1, {8, 4, 8}, Block::air);
}

static void placeCactus(Chunk& chunk, WorldGenRNG& rng, glm::ivec3 local) {
    int cactusHeight = rng.getInt(1, 4);
    local += glm::ivec3{0, 1, 0};
    if (chunk.getBlock(local) != Block::air) {
        return;
    }

    for (int i = 0; i < cactusHeight; ++i) {
        chunk.setBlockRaw(local + glm::ivec3{0, i, 0}, Block::cactus);
    }
}

void placeOakTree(Chunk& chunk, WorldGenRNG& rng, glm::ivec3 local) {
    static const glm::ivec3 corners[4] = {
        {1, 0, 1},
        {-1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
    };

    chunk.setBlockRaw(local, Block::dirt);
    local += glm::ivec3{0, 1, 0};

    int height = rng.getInt(5, 7);
    chunk.placeBoxRaw(local, {1, height, 1}, Block::log);

    //worldTryPlaceBox(world, pointAddValue(worldPoint, -2, 2, -2), point(5, 2, 5), Block::leaves);

    chunk.tryPlaceBoxRaw(local + glm::ivec3{-2, height - 3, -1}, {5, 2, 3}, Block::leaves);
    chunk.tryPlaceBoxRaw(local + glm::ivec3{-1, height - 3, -2}, {3, 2, 5}, Block::leaves);

    for (int i = 0; i < 4; ++i) {
        glm::ivec3 scaledCorner = corners[i] * 2;
        for (int j = height - 3; j < height - 1; ++j) {
            if (!rng.chance(1, 2)) {
                continue;
            }
            chunk.tryPlaceBlockRaw(local + scaledCorner + glm::ivec3{0, j, 0}, Block::leaves);
        }
    }

    chunk.tryPlaceBoxRaw(local + glm::ivec3{-1, height - 1, 0}, {3, 2, 1}, Block::leaves);
    chunk.tryPlaceBoxRaw(local + glm::ivec3{0, height - 1, -1}, {1, 2, 3}, Block::leaves);

    for (int i = 0; i < 4; ++i) {
        if (!rng.chance(1, 2)) {
            continue;
        }

        chunk.tryPlaceBlockRaw(local + corners[i] + glm::ivec3{0, height - 1, 0}, Block::leaves);
    }
}

void placeSpruceTree(Chunk& chunk, WorldGenRNG& rng, glm::ivec3 local) {
    chunk.setBlockRaw(local, Block::dirt);
    local += glm::ivec3{0, 1, 0};

    Block trunk = Block::log;
    Block leaves = Block::leaves;

    int layerCount = rng.getInt(1, 5);

    int height = layerCount * 2 + 2 + rng.getInt(1, 3);

    chunk.placeBoxRaw(local, {1, height, 1}, trunk);
    chunk.tryPlaceBlockRaw(local + glm::ivec3{0, height, 0}, leaves);
    for (Direction direction : cardinalDirections) {
        chunk.tryPlaceBlockRaw(local + glm::ivec3{0, height - 1, 0} + directionToIvec3(direction), leaves);
    }

    for (int i = 0; i < layerCount; ++i) {
        int y = height - 3 - i * 2;

        for (Direction direction : cardinalDirections) {
            chunk.tryPlaceBlockRaw(local + glm::ivec3{0, y, 0} + directionToIvec3(direction), leaves);
        }

        chunk.tryPlaceBoxRaw(local + glm::ivec3{-1, y - 1, -2}, glm::ivec3{3, 1, 5}, leaves);
        for (int z = -1; z <= 1; ++z) {
            chunk.tryPlaceBlockRaw(local + glm::ivec3{-2, y - 1, z}, leaves);
            chunk.tryPlaceBlockRaw(local + glm::ivec3{2, y - 1, z}, leaves);
        }
    }
}

void placeTree(Chunk& chunk, WorldGenRNG& rng, glm::ivec3 local) {
    Logger::debug("Placing Tree");

    Block surfaceBlock = chunk.getBlock(local);

    switch (surfaceBlock) {
        case Block::sand:
            placeCactus(chunk, rng, local);
            break;
        case Block::grass:
            placeOakTree(chunk, rng, local);
            break;
        case Block::snowyGrass:
            placeSpruceTree(chunk, rng, local);
            break;
        default:
            break;
    }
}

static inline float noiseLayer2D(int worldSeed, glm::ivec3 position, int layerIndex, float scale, int octaves) {
    glm::vec3 scaledPosition = glm::vec3{position} / scale;
    float ySeed = -100.f * layerIndex - worldSeed;

    return stb_perlin_fbm_noise3(scaledPosition.x, ySeed, scaledPosition.z, 2.f, 0.5f, octaves);
}

void generateTerrain(Chunk* chunk) {
    Logger::assertion(chunk);

    //World* world = chunk->world;
    glm::ivec3 coords = chunk->coords;

    int worldSeed = chunk->world->seed;

    ITERATE_CHUNK(x, y, z) {
        chunk->blocks[x][y][z] = Block::air;
    }

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            int layerIndex = 1;

            int surface = surfaceOffset;

            //float biomeScale = 0.001f;

            float wx = x + CHUNK_WIDTH * coords.x;
            float wz = z + CHUNK_WIDTH * coords.z;
            glm::ivec3 position = localToWorld(coords, glm::ivec3{x, 0, z});
            
            bool useBiomes = !Config::settings->world.superflat;

            float temperature = 0.f;
            float humidity = 0.f;
            if (useBiomes) {
                temperature = noiseLayer2D(worldSeed, position, layerIndex++, 1000, 10);
                humidity = noiseLayer2D(worldSeed, position, layerIndex++, 1000, 10);
            }

            chunk->averageTemperature += temperature;
            chunk->averageHumidity += humidity;

            if (!Config::settings->world.superflat) {
                float scale = 250.f;
                float stretch = 32.f;
                int octaves = 15;
                float noise = noiseLayer2D(worldSeed, position, layerIndex++, scale, octaves);
                // float noise = stb_perlin_fbm_noise3(wx * scale, wz * scale, -200.f - worldSeed, 2.f, 0.5f, octaves);
                surface = noise * stretch + surfaceOffset;
            }
            

            float riverNoise = noiseLayer2D(worldSeed, position, layerIndex++, 500, 8);
            bool isRiver = riverNoise > 0.1f && riverNoise < 0.2f;
            float riverAmount = (0.05f - std::abs(riverNoise - 0.15f)) / 0.05f * 20.f;

            if (isRiver) {
                surface = std::min(surface, static_cast<int>(surface - riverAmount));
            }
            
            
            float mountainThreshold = 0.5f;
            float mountainScale = 100.f;
            float mountainNoise = noiseLayer2D(worldSeed, position, layerIndex++, 1000, 8);
            chunk->averageGeology += mountainNoise;
            
            surface = (surface - surfaceOffset) * std::max(0.5f, 1.f + mountainNoise) + surfaceOffset;
            
            if (mountainNoise > mountainThreshold) {
                float mountainAmount = (mountainNoise - mountainThreshold) / mountainThreshold * mountainScale;
                surface += mountainAmount;
            }
            
            float plateauThreshold = 0.5f;
            int plateauHeight = 10;
            float plateauNoise = noiseLayer2D(worldSeed, position, layerIndex++, 125, 2);
            
            if (plateauNoise > plateauThreshold) {
                surface += plateauHeight;
            }

            surface = clampInt(surface, 1, CHUNK_HEIGHT - 1);

            chunk->surfaceHeight[x][z] = surface;

            chunk->blocks[x][0][z] = Block::bedrock;

            Block topLayerBlock = Block::dirt;
            Block surfaceBlock = Block::grass;
            Block underwaterBlock = Block::sand;

            if (temperature < 0) {
                if (humidity > 0) {
                    surfaceBlock = Block::snowyGrass;
                } else {
                    surfaceBlock = Block::grass;
                }
                underwaterBlock = Block::gravel;
            } else {
                if (humidity > 0) {
                    surfaceBlock = Block::grass;
                } else {
                    topLayerBlock = Block::sand;
                    surfaceBlock = Block::sand;
                }
            }


            int beachSize = std::max(0.f, -humidity * 6);

            if (surface < seaLevel + beachSize) {
                topLayerBlock = underwaterBlock;
                surfaceBlock = underwaterBlock;
            }

            for (int y = 1; y <= surface; ++y) {
                if (Config::settings->world.generateCaves) {
                    float caveThreshold = -0.5f;
                    float caveScale = 0.05;
                    float caveNoise = stb_perlin_fbm_noise3(y * caveScale, wz * caveScale + worldSeed, wx * caveScale, 2.f, 0.5f, 4);

                    if (caveNoise < caveThreshold) {
                        continue;
                    }
                }

                Block block = topLayerBlock;
                if (y < surface - dirtLayer) {
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

            if (surface < seaLevel) {
                for (int y = seaLevel; y > 0; --y) {
                    if (chunk->blocks[x][y][z] != Block::air) {
                        break;
                    }

                    Block waterBlock = Block::water;
                    if (y == seaLevel && useBiomes && temperature < 0.f && humidity > 0.f) {
                        waterBlock = Block::ice;
                    }
                    chunk->blocks[x][y][z] = waterBlock;
                }
            }


            //chunk->blocks[x][surface][z] = surfaceBlock;
        }
    }

    chunk->averageTemperature /= CHUNK_WIDTH * CHUNK_WIDTH;
    chunk->averageHumidity /= CHUNK_WIDTH * CHUNK_WIDTH;
    chunk->averageGeology /= CHUNK_WIDTH * CHUNK_WIDTH;

    chunk->dirty = true;
}

void placeFeatures(Chunk* chunk) {
    if (!Config::settings->world.generateFeatures) {
        return;
    }

    WorldGenRNG rng{chunk->world->seed, chunk->coords};

    int maxTrees = (chunk->averageHumidity + 1.f) * 7;
    int treeCount = rng.getInt(0, maxTrees);

    for (int i = 0; i < treeCount; ++i) {
        glm::ivec3 point = rng.getSurfacePosition(chunk, 2);

        placeTree(*chunk, rng, point);
    }

    if (rng.chance(1, 20)) {
        placeDungeon(*chunk, {rng.getInt(CHUNK_WIDTH - 10), 10, rng.getInt(CHUNK_WIDTH - 10)});
    }

    int grassCount = rng.getInt(0, 150);
    for (int i = 0; i < grassCount; ++i) {
        glm::ivec3 grassPosition = rng.getSurfacePosition(chunk);

        if (chunk->getBlock(grassPosition) == Block::grass) {
            chunk->tryPlaceBlockRaw(grassPosition + glm::ivec3{0, 1, 0}, Block::tallGrass);
        }
    }

    int flowerCount = rng.getInt(-10, 10);

    Block flowerType = rng.chance(1, 2) ? Block::rose : Block::dandelion;
    for (int i = 0; i < flowerCount; ++i) {
        glm::ivec3 flowerPosition = rng.getSurfacePosition(chunk);

        if (chunk->getBlock(flowerPosition) == Block::grass) {
            chunk->tryPlaceBlockRaw(flowerPosition + glm::ivec3{0, 1, 0}, flowerType);
        }
    }

    if (chunk->averageHumidity < 0.f && chunk->averageTemperature > 0.f) {
        int bushCount = rng.getInt(-2, 3);
        for (int i = 0; i < bushCount; ++i) {
            glm::ivec3 bushPosition = rng.getSurfacePosition(chunk);

            if (chunk->getBlock(bushPosition) == Block::sand) {
                chunk->tryPlaceBlockRaw(bushPosition + glm::ivec3{0, 1, 0}, Block::deadBush);
            }
        }
    }

    chunk->dirty = true;
}

