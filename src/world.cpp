#include <memory>
#include <stdint.h>
#include "world.hpp"
#include "entity.hpp"
#include "util.hpp"
#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "serialize.hpp"
#include "logger.hpp"

static int chunkDistance(glm::ivec3 from, glm::ivec3 to) {
    return (int)floorf(sqrtf(squaref(from.x - to.x) + squaref(from.z - to.z)));
}


World::World() {
    glm::vec3 playerPosition{
        0.f,
        SURFACE_OFFSET + 10.f, // worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + PLAYER_EYE + 2.f,
        0.f
    };

    player = &spawnEntity<Player>(playerPosition); // spawnEntity(world, ENTITY_PLAYER, playerPosition, 0.6f, 1.8f);

    if (loadWorld(this)) {
        return;
    }

#ifndef DEFAULT_SET_SEED
    seed = randomInt(10000);
#endif

    /*const int renderDistance = 1;

    for (int x = -renderDistance; x <= renderDistance; ++x) {
        for (int z = -renderDistance; z <= renderDistance; ++z) {
            Point point = {x, 0, z};

            chunkInit(world, point);
        }
    }*/
}

World::~World() {
    saveWorld(this);
    chunks.clear();
}

void World::update(float deltaTime) {
    static int lightDirection = 1;

    if (skyLight > 1.f) {
        lightDirection = -1;
    }

    if (skyLight < 0.f) {
        lightDirection = 1;
    }

    skyLight += lightDirection * deltaTime * 0.1;
    skyLight = 1.f;

    for (auto& entity : entities) {
        entity->update(deltaTime);
    }

    if (player) {
        int maxChunkLoads = 2;

        glm::ivec3 chunkOffset{0};
        CircleIterator offsetIterator = circleIteratorInit(renderDistance);

        do {
            glm::ivec3 chunkCoord = chunkOffset + worldToChunkV(player->position);
            chunkCoord.y = 0;
            Chunk* chunk = getChunk(chunkCoord);

            if (chunk != nullptr) {
                continue;
            }

            int distance = chunkDistance(worldToChunkV(player->position), chunkCoord);
            if (distance > renderDistance) {
                continue;
            }

            if (maxChunkLoads <= 0 && distance != 0) {
                continue;
            }
            maxChunkLoads--;
            
            std::unique_ptr<Chunk> newChunk = std::make_unique<Chunk>(this, chunkCoord);
            chunks[chunkCoord] = std::move(newChunk);
        } while (iterateCircleIterator(&offsetIterator, &chunkOffset));
    }

    std::vector<Chunk*> toUnload{};

    for (auto& entry : chunks) {
        Chunk* chunk = entry.second.get();

        glm::ivec3 playerChunk = worldToChunkV(player->position);

        int distance = chunkDistance(playerChunk, chunk->coords);

        if (distance > renderDistance + 1) {
            toUnload.push_back(chunk);
        }
    }

    for (Chunk* chunk : toUnload) {
        chunks.erase(chunk->coords);
    }

    for (auto& entry : chunks) {
        Chunk* chunk = entry.second.get();

        if (chunk->dirty) {
            chunkGenerateMesh(chunk);
        }
    }
}

void World::draw(ShaderProgram& terrainShader, ShaderProgram& entityShader) const {
    terrainShader.setUniformFloat("skyLight", skyLight);
    terrainShader.setUniformVec4("fogColor", skyColor);

    //float fogDropoff = 4000.f;
    //float fogDistance = 5000.f;
    
    float fogDropoff = (renderDistance - 1) * 16;
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
    CircleIterator offsetIterator = circleIteratorInit(renderDistance + 1);
    do {
        glm::ivec3 chunkCoord = chunkOffset + worldToChunkV(player->position);
        chunkCoord.y = 0;
        const Chunk* chunk = getChunk(chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        chunk->draw(terrainShader);
        chunk->drawTranslucent(terrainShader);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));

    //blendModeNormal();
    /*offsetIterator = circleIteratorInit(renderDistance + 1);
    do {
        Point chunkCoord = pointAdd(chunkOffset, worldToChunkV(player->position));
        chunkCoord.y = 0;
        Chunk* chunk = worldGetChunk(world, chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        drawChunkTranslucent(chunk, terrainShader);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));*/

    entityShader.use();
    for (auto& entity : entities) {
        entity->draw(entityShader);
    }
}

Block World::getBlock(glm::ivec3 worldPoint) const {
    const Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == NULL) {
        //return BLOCK_BARRIER;
        return BLOCK_AIR;
    }
    return chunk->getBlockRaw(worldToLocal(worldPoint));
}

void World::setBlock(glm::ivec3 worldPoint, Block block) {
    Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == NULL) {
        Logger::info("Block attempted to be placed outside of loaded chunks");
        return;
    }
    chunk->setBlock(worldToLocal(worldPoint), block);
}

Chunk* World::getChunk(glm::ivec3 chunkCoords) {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    return chunks.at(chunkCoords).get();
}

const Chunk* World::getChunk(glm::ivec3 chunkCoords) const {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    return chunks.at(chunkCoords).get();
}

void World::markDirty(glm::ivec3 worldPoint) {
    Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }
    chunk->markDirty(worldToLocal(worldPoint));
}

void World::tryPlaceBlock(glm::ivec3 worldPoint, Block block) {
    Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }

    glm::ivec3 local = worldToLocal(worldPoint);
    chunk->tryPlaceBlock(local.x, local.y, local.z, block);
}

void World::tryPlaceBox(glm::ivec3 start, glm::ivec3 size, Block block) {
    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                tryPlaceBlock(glm::ivec3{x, y, z}, block);
            }
        }
    }
}

void World::placeBox(glm::ivec3 start, glm::ivec3 size, Block block) {
    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                setBlock(glm::ivec3{x, y, z}, block);
            }
        }
    }
}


int shaderModelUniform = 0;
int shaderSkylight = 0;
int shaderFogColor = 0;
int shaderFogDistance = 0;
int shaderFogDropoff = 0;



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
