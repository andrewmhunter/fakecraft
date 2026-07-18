#include <future>
#include <glm/fwd.hpp>
#include <memory>
#include "world.hpp"
#include "entities/entity.hpp"
#include "graphics/graphics.hpp"
#include "engine/resource_manager.hpp"
#include "level/octree.hpp"
#include "util/util.hpp"
#include "chunk.hpp"
#include "chunk_mesh.hpp"
#include "serialization/serialize.hpp"
#include "engine/logger.hpp"

static int chunkDistance(glm::ivec3 from, glm::ivec3 to) {
    return (int)floorf(sqrtf(squaref(from.x - to.x) + squaref(from.z - to.z)));
}


World::World()
    : currentEntityID{0},
    seed{Config::settings->world.setSeed.value_or(randomInt(10000))},
    renderDistance{Config::settings->graphics.renderDistance}
{
    glm::vec3 playerPosition{
        0.f,
        70.f + 10.f,
        0.f
    };

    player = &spawnEntity<Player>(playerPosition);

    if (loadWorld(this)) {
        return;
    }

    Logger::info(std::format("World seed: {}", seed));
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

    
    
    collisionWorld = CollisionWorld{};
    for (auto& entity : entities) {
        collisionWorld.insertEntity(entity.first, entity.second->getBoundingBox());
    }

    for (auto& entity : entities) {
        auto collisions = collisionWorld.getCollisions(entity.second->getBoundingBox(), entity.first);
        entity.second->update(deltaTime);
        for (EntityID other : collisions) {
            entity.second->collide(deltaTime, other);
        }
    }

    if (player) {
        int maxChunkLoads = 2;

        glm::ivec3 chunkOffset{0};
        CircleIterator offsetIterator = circleIteratorInit(renderDistance);

        std::vector<std::future<void>> asyncGenerators{};

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

            asyncGenerators.push_back(std::async([this, chunkCoord](){
                this->chunks[chunkCoord]->generateOrLoad();
            }));
        } while (iterateCircleIterator(&offsetIterator, &chunkOffset));

        for (auto& handle : asyncGenerators) {
            handle.get();
        }
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

void World::draw() const {
    ShaderProgram& terrainShader = ResourceManager::instance().shader.terrainShader;
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
    ResourceManager::instance().texture.terrain.bind();

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
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));

    ShaderProgram& entityShader = ResourceManager::instance().shader.entityShader;
    entityShader.use();
    ResourceManager::instance().texture.human.bind();
    for (auto& entity : entities) {
        entity.second->draw(entityShader);
    }

    terrainShader.use();
    ResourceManager::instance().texture.terrain.bind();

    glDisable(GL_CULL_FACE);
    chunkOffset = glm::ivec3{0};
    offsetIterator = circleIteratorInit(renderDistance + 1);
    do {
        glm::ivec3 chunkCoord = chunkOffset + worldToChunkV(player->position);
        chunkCoord.y = 0;
        const Chunk* chunk = getChunk(chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        chunk->drawTranslucent(terrainShader);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));
    glEnable(GL_CULL_FACE);
}

Block World::getBlock(glm::ivec3 worldPoint) const {
    const Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == nullptr) {
        return Block::air;
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

    return true;
}
