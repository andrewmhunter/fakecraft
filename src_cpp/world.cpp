#include <raymath.h>
#include <stdint.h>
#include "world.hpp"
#include "util.hpp"
#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "serialize.hpp"
#include "logger.hpp"

void worldTryPlaceBox(World* world, glm::ivec3 start, glm::ivec3 size, Block block) {
    ASSERT(world);

    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                worldTryPlaceBlock(world, (glm::ivec3){x, y, z}, block);
            }
        }
    }
}

void worldPlaceBox(World* world, glm::ivec3 start, glm::ivec3 size, Block block) {
    ASSERT(world);

    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                worldSetBlock(world, (glm::ivec3){x, y, z}, block);
            }
        }
    }
}

void worldInit(World* world) {
    ASSERT(world);

    world->renderDistance = DEFAULT_RENDER_DISTANCE;
    world->showChunkBorders = false;
    world->skyLight = 0.f;
    world->skyColor = color::skyblue;

    world->chunks = {};

    glm::vec3 playerPosition{
        0.f,
        SURFACE_OFFSET + 10.f, // worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + PLAYER_EYE + 2.f,
        0.f
    };

    world->player = spawnEntity(world, ENTITY_PLAYER, playerPosition, 0.6f, 1.8f);

    if (loadWorld(world)) {
        return;
    }

#ifdef DEFAULT_SET_SEED
    world->seed = DEFAULT_SET_SEED;
#else
    world->seed = randomInt(10000);
#endif

    /*const int renderDistance = 1;

    for (int x = -renderDistance; x <= renderDistance; ++x) {
        for (int z = -renderDistance; z <= renderDistance; ++z) {
            Point point = {x, 0, z};

            chunkInit(world, point);
        }
    }*/
}

void worldUnload(World* world) {
    saveWorld(world);

    for (auto entry : world->chunks) {
        Chunk* chunk = entry.second;
        chunkUnload(chunk);
        delete chunk;
    }

    for (size_t i = 0; i < world->entities.size(); ++i) {
        entityUnload(world->entities[i]);
    }
}

int shaderModelUniform = 0;
int shaderSkylight = 0;
int shaderFogColor = 0;
int shaderFogDistance = 0;
int shaderFogDropoff = 0;

static int chunkDistance(glm::ivec3 from, glm::ivec3 to) {
    return (int)floorf(sqrtf(squaref(from.x - to.x) + squaref(from.z - to.z)));
}


CircleIterator circleIteratorInit(int distance) {
    return (CircleIterator) {
        .distance = distance,
        .row = 0,
        .direction = static_cast<Direction>(DIRECTION_CARDINAL_COUNT),
        .column = 0,
        .side = 1,
    };
}

static bool updateStateCircleIterator(CircleIterator* state) {
    if (state->side == 1) {
        state->side = -1;
    } else if (state->side == -1) {
        if (state->column != state->row) {
            state->side = 1;
            return true;
        }
    }

    state->direction += 1;
    if (state->direction < DIRECTION_CARDINAL_COUNT) {
        return true;
    }
    state->direction = 0;

    state->column += 1;

    state->side = -1;
    if (state->column < state->row + 1) {
        return true;
    }
    state->column = 0;

    state->row += 1;
    state->side = 0;
    if (state->row < state->distance + 1) {
        return true;
    }

    return false;
}

bool iterateCircleIterator(CircleIterator* state, glm::ivec3* pointOut) {
    if (!updateStateCircleIterator(state)) {
        return false;
    }

    glm::ivec3 directionPoint = directionToPoint(static_cast<Direction>(state->direction));
    glm::ivec3 rightAnglePoint = directionToPoint(directionCardinalRightAngle(static_cast<Direction>(state->direction)));

    glm::ivec3 point = directionPoint * state->row;

    glm::ivec3 offsetColumn = rightAnglePoint * state->side * state->column;
    point = point + offsetColumn;

    *pointOut = point;

    //fprintf(stderr, "% d % d\n", point.x, point.z);

    return true;
}

void worldUpdate(World* world, float deltaTime) {
    ASSERT(world);

    static int lightDirection = 1;

    if (world->skyLight > 1.f) {
        lightDirection = -1;
    }

    if (world->skyLight < 0.f) {
        lightDirection = 1;
    }

    world->skyLight += lightDirection * deltaTime * 0.1;
    world->skyLight = 1.f;

    for (Entity* entity : world->entities) {
        entityUpdate(entity, deltaTime);
    }

    int renderDistance = world->renderDistance;

    if (world->player) {
        int maxChunkLoads = 2;

        glm::ivec3 chunkOffset{0};
        CircleIterator offsetIterator = circleIteratorInit(renderDistance);

        do {
            glm::ivec3 chunkCoord = chunkOffset + worldToChunkV(world->player->position);
            chunkCoord.y = 0;
            Chunk* chunk = worldGetChunk(world, chunkCoord);

            if (chunk != NULL) {
                continue;
            }

            int distance = chunkDistance(worldToChunkV(world->player->position), chunkCoord);
            if (distance > renderDistance) {
                continue;
            }

            if (maxChunkLoads <= 0 && distance != 0) {
                continue;
            }
            maxChunkLoads--;

            chunkInit(world, chunkCoord);
        } while (iterateCircleIterator(&offsetIterator, &chunkOffset));
    }

    std::vector<Chunk*> toUnload{};

    for (auto entry : world->chunks) {
        Chunk* chunk = entry.second;

        glm::ivec3 playerChunk = worldToChunkV(world->player->position);

        int distance = chunkDistance(playerChunk, chunk->coords);

        if (distance > renderDistance + 1) {
            toUnload.push_back(chunk);
        }
    }

    for (Chunk* chunk : toUnload) {
        chunkUnload(chunk);
        world->chunks.erase(chunk->coords);
        delete chunk;
    }

    for (auto entry : world->chunks) {
        Chunk* chunk = entry.second;

        if (chunk->dirty) {
            chunkGenerateMesh(chunk);
        }
    }
}

void worldDraw(World* world, Shader terrainShader, Shader entityShader) {
    ASSERT(world);

    terrainShader.setUniformFloat("skyLight", world->skyLight);
    terrainShader.setUniformVec4("fogColor", world->skyColor);

    //float fogDropoff = 4000.f;
    //float fogDistance = 5000.f;
    
    float fogDropoff = (world->renderDistance - 1) * 16;
    fogDropoff *= fogDropoff;
    float fogDistance = fogDropoff * 4.f / 5.f;


    terrainShader.setUniformFloat("fogDistance", fogDistance);
    terrainShader.setUniformFloat("fogDropoff", fogDropoff);

    terrainShader.use();

    /*HashEntry* chunkIt = NULL;
    while (setIterate(&world->chunks, &chunkIt)) {
        Chunk* chunk = chunkIt->contents;

        drawChunk(chunk, material);
    }*/

    //blendModeReplace();
    glm::ivec3 chunkOffset{0};
    CircleIterator offsetIterator = circleIteratorInit(world->renderDistance + 1);
    do {
        glm::ivec3 chunkCoord = chunkOffset + worldToChunkV(world->player->position);
        chunkCoord.y = 0;
        Chunk* chunk = worldGetChunk(world, chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        drawChunk(chunk, terrainShader);
        drawChunkTranslucent(chunk, terrainShader);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));

    //blendModeNormal();
    /*offsetIterator = circleIteratorInit(world->renderDistance + 1);
    do {
        Point chunkCoord = pointAdd(chunkOffset, worldToChunkV(world->player->position));
        chunkCoord.y = 0;
        Chunk* chunk = worldGetChunk(world, chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        drawChunkTranslucent(chunk, terrainShader);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));*/

    entityShader.use();
    for (const Entity* entity : world->entities) {
        entityDraw(entityShader, entity);
    }
}

void worldMarkDirty(World* world, glm::ivec3 worldPoint) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }
    chunkMarkDirty(chunk, worldToLocal(worldPoint));
}

bool chunkCoordsInArray(const World* world, glm::ivec3 chunkCoords) {
    // Hide unused parameter warnings
    (void)world;

    static const int limit = WORLD_MAX_CHUNK_WIDTH / 2;
    return chunkCoords.x >= -limit
        && chunkCoords.x < limit
        && chunkCoords.z >= -limit
        && chunkCoords.z < limit
        && chunkCoords.y == 0;
}

const Chunk* worldGetChunkConst(const World* world, glm::ivec3 chunkCoords) {
    ASSERT(world);

    if (world->chunks.count(chunkCoords) == 0) {
        return nullptr;
    }

    return world->chunks.at(chunkCoords);
}

Chunk* worldGetChunk(World* world, glm::ivec3 chunkCoords) {
    ASSERT(world);

    if (world->chunks.count(chunkCoords) == 0) {
        return nullptr;
    }

    return world->chunks.at(chunkCoords);
}

Block worldGetBlock(const World* world, glm::ivec3 worldPoint) {
    //ASSERT(world);

    const Chunk* chunk = worldGetChunkConst(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        //return BLOCK_BARRIER;
        return BLOCK_AIR;
    }
    return chunkGetBlockRaw(chunk, worldToLocal(worldPoint));
}

void worldSetBlock(World* world, glm::ivec3 worldPoint, Block block) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        INFO("Block attempted to be placed outside of loaded chunks");
        return;
    }
    chunkSetBlock(chunk, worldToLocal(worldPoint), block);
}

void worldTryPlaceBlock(World* world, glm::ivec3 worldPoint, Block block) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }

    glm::ivec3 local = worldToLocal(worldPoint);
    chunkTryPlaceBlock(chunk, local.x, local.y, local.z, block);
}


Entity* spawnEntity(World* world, EntityType type, glm::vec3 position, float width, float height) {
    ASSERT(world);

    Entity* entity = new Entity;
    ASSERT(entity);

    world->entities.push_back(entity);
    entityInit(entity, type, world, position, width, height);
    return entity;
}

