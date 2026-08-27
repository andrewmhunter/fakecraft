#include <cmath>
#include <filesystem>
#include <format>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <optional>
#include "world.hpp"
#include "engine/camera.hpp"
#include "engine/config.hpp"
#include "entities/entity.hpp"
#include "entities/entity_model.hpp"
#include "graphics/graphics.hpp"
#include "engine/resource_manager.hpp"
#include "engine/input.hpp"
#include "level/collision.hpp"
#include "level/octree.hpp"
#include "serialization/serialize.hpp"
#include "util/thread_pool.hpp"
#include "util/types.hpp"
#include "util/util.hpp"
#include "chunk.hpp"
#include "engine/logger.hpp"

static int chunkDistance(glm::ivec3 from, glm::ivec3 to) {
    return std::floorf(std::sqrtf(squaref(from.x - to.x) + squaref(from.z - to.z)));
}

World::World()
    : currentEntityID{0},
    threadPool{Config::settings->game.threadCount},
    seed{Config::settings->world.setSeed.value_or(randomInt(10000))},
    renderDistance{Config::settings->graphics.renderDistance}
{
    glm::vec3 playerPosition{
        0.f,
        70.f + 10.f,
        0.f
    };

    player = &spawnEntity<Player>(playerPosition);

    if (deserialize()) {
        return;
    }

    Logger::info(std::format("World seed: {}", seed));
}

World::~World() {
    // The threadpool must be terminated before any fields are destructed as
    // the running tasks might reference those fields
    threadPool.terminate();
    serialize();
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

    int adjustedRenderDistance = renderDistance + 1;

    if (player) {
        int maxChunkLoads = 1024;

        for (int x = -adjustedRenderDistance; x <= adjustedRenderDistance; ++x) {
            for (int z = -adjustedRenderDistance; z <= adjustedRenderDistance; ++z) {

                glm::ivec3 chunkCoord = glm::ivec3{x, 0, z} + worldToChunkV(player->position);
                chunkCoord.y = 0;
                Chunk* chunk = getChunkRaw(chunkCoord);

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
                auto& thisChunk = *chunks[chunkCoord];

                threadPool.enqueueTask(-distance, [&thisChunk](){
                    thisChunk.generateOrLoad();
                    thisChunk.state = ChunkState::terrainGenerated;
                });

            }
        }
    }

    std::vector<Chunk*> toUnload{};

    for (auto& entry : chunks) {
        Chunk* chunk = entry.second.get();

        if (chunk->state != ChunkState::loaded) {
            continue;
        }

        glm::ivec3 playerChunk = worldToChunkV(player->position);

        int distance = chunkDistance(playerChunk, chunk->coords);

        if (distance > renderDistance + 1) {
            toUnload.push_back(chunk);
        }
    }

    for (Chunk* chunk : toUnload) {
        chunks.erase(chunk->coords);
    }

    int maxMeshUploads = 8;

    for (auto& entry : chunks) {
        Chunk* chunk = entry.second.get();
        switch (chunk->state) {
            case ChunkState::terrainGenerated:
                chunk->fillChunkCache();
                chunk->state = ChunkState::waitingForAdjacents;
                [[fallthrough]];
            case ChunkState::waitingForAdjacents: {
                if (!chunk->adjacentCardinalsAtLeastInState(ChunkState::terrainGenerated)) {
                    break;
                }
                chunk->state = ChunkState::generatingInitialMesh;
                chunk->dirty = true;
                int priority = -chunkDistance(worldToChunkV(player->position), chunk->coords);
                threadPool.enqueueTask(priority, [chunk](){
                    chunk->generateMesh();
                    chunk->state = ChunkState::initialMeshGenerated;
                });
                break;
            }
            case ChunkState::initialMeshGenerated:
                if (maxMeshUploads-- <= 0) {
                    continue;
                }
                chunk->uploadMesh();
                chunk->state = ChunkState::loaded;
                break;
            default:
                break;
        }  
    }

    int regeneratedMeshes = 0;
    for (auto& entry : chunks) {
        Chunk* chunk = entry.second.get();

        if (chunk->state != ChunkState::loaded) {
            continue;
        }

        if (chunk->dirty) {
            regeneratedMeshes++;
            chunk->generateMesh();
            chunk->uploadMesh();
        }
    }

    if (regeneratedMeshes > 0) {
        //Logger::info(std::format("Initial {} regenerated {}", initialGeneratedMeshes, regeneratedMeshes));
    }
}

void World::draw() const {
    ShaderProgram& terrainShader = ResourceManager::instance().shader.terrain;
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

    Frustrum frustrum = globalCamera.getFrustrum();
    //int chunksCulled = 0;

    for (const auto& chunkIt : chunks) {
        const Chunk* chunk = chunkIt.second.get();
        if (chunk == nullptr || chunk->state != ChunkState::loaded) {
            continue;
        }

        if (!frustrum.isInFrustrum(chunk->getCullBoundingBox())) {
            //chunksCulled++;
            if (!keyDown(GLFW_KEY_F7)) {
                continue;
            }
        }

        chunk->draw(terrainShader);
    }

    
    ShaderProgram& entityShader = ResourceManager::instance().shader.entity;
    entityShader.use();
    ResourceManager::instance().texture.human.bind();

    //int entitiesCulled = 0;

    std::map<EntityDrawFeatures, std::vector<Bones>> entitiesToDraw{};

    entityShader.setModel(glm::mat4{1.f});
    for (auto& entity : entities) {
        if (!frustrum.isInFrustrum(entity.second->getCullBoundingBox())) {
            //entitiesCulled++;
            continue;
        }
        entity.second->appendDrawCommands(entitiesToDraw);
    }

    GLint bonesUniformLocation = entityShader.uniformLocation("bones");

    for (const auto& entityKind : entitiesToDraw) {
        entityKind.first.first->bind();

        entityKind.first.second->bind();

        for (const Bones& bones : entityKind.second) {
            glUniformMatrix4fv(bonesUniformLocation, maxBones, false, glm::value_ptr(bones[0]));
            entityKind.first.second->draw();
        }
    }

    //Logger::info(std::format("Chunks culled: {}, Entities culed: {}", chunksCulled, entitiesCulled));

    terrainShader.use();
    ResourceManager::instance().texture.terrain.bind();

    glDisable(GL_CULL_FACE);
    for (const auto& chunkIt : chunks) {
        const Chunk* chunk = chunkIt.second.get();

        if (chunk == nullptr || chunk->state != ChunkState::loaded) {
            continue;
        }

        if (!frustrum.isInFrustrum(chunk->getCullBoundingBox())) {
            continue;
        }

        chunk->drawTranslucent(terrainShader);
    }
    glEnable(GL_CULL_FACE);
}

Block World::getBlock(glm::ivec3 worldPoint) const {
    const Chunk* chunk = getChunk(worldToChunk(worldPoint));
    if (chunk == nullptr || worldPoint.y < 0 || worldPoint.y >= CHUNK_HEIGHT) {
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

Chunk* World::getChunkRaw(glm::ivec3 chunkCoords) {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    Chunk* chunk = chunks.at(chunkCoords).get();
    return chunk;
}

const Chunk* World::getChunkRaw(glm::ivec3 chunkCoords) const {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    const Chunk* chunk = chunks.at(chunkCoords).get();
    return chunk;
}

Chunk* World::getChunk(glm::ivec3 chunkCoords, ChunkState atLeastState) {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    Chunk* chunk = chunks.at(chunkCoords).get();
    if (!chunk->atLeastInState(atLeastState)) {
        return nullptr;
    }
    return chunk;
}

const Chunk* World::getChunk(glm::ivec3 chunkCoords, ChunkState atLeastState) const {
    if (chunks.count(chunkCoords) == 0) {
        return nullptr;
    }
    const Chunk* chunk = chunks.at(chunkCoords).get();
    if (!chunk->atLeastInState(atLeastState)) {
        return nullptr;
    }
    return chunk;
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


Entity& World::spawnEntity(EntityType type, glm::vec3 position) {
    EntityID id = currentEntityID++;
    return spawnEntity(type, id, position);
}

Entity& World::spawnEntity(EntityType type, EntityID id, glm::vec3 position) {
    std::unique_ptr<Entity> entity = entityFactory(this, type, id, position);
    Entity& entityRef = *entity.get();
    entities[id] = std::move(entity);
    return entityRef;
}

static std::filesystem::path worldFileName() {
    return std::format("{}/world.bin", Config::settings->world.saveFile);
} 

void World::serialize() {
    if (!Config::settings->game.saveChunks) {
        return;
    }

    ser::Object object{};
    serializeDeserialize(object);

    ser::Object playerObject{};
    player->serialize(playerObject);
    object.setField("player", playerObject);

    std::vector<ser::Object> ents{};
    for (auto& [id, entity] : entities) {
        if (entity->type == EntityType::player) {
            continue;
        }

        ser::Object entityObject{};
        entity->serialize(entityObject);
        ents.push_back(entityObject);
    }
    object.setField("entities", ser::List{ents});

    try {
        ser::serialize(worldFileName(), ser::Dynamic{object});
    } catch (ser::Error& ex) {
        Logger::error(std::format("While serializing '{}': ", worldFileName().c_str(), ex.what()));
    }
}

bool World::deserialize() {
    if (!Config::settings->game.loadChunks) {
        return false;
    }

    std::filesystem::path fileName = worldFileName();
    if (!std::filesystem::is_regular_file(fileName)) {
        return false;
    }

    try {
        ser::Object object = ser::deserialize(fileName).get<ser::Object>();
        serializeDeserialize(object);
    
        player->deserialize(object.getField<ser::Object>("player"));
    
        std::vector<ser::Object>& ents = object.getField<ser::List>("entities").getVector<ser::Object>();
        for (ser::Object& entityObject : ents) {
            EntityType type = static_cast<EntityType>(entityObject.getField<i32>("type"));
            EntityID id{entityObject.getField<u64>("id")};
            Entity& entity = spawnEntity(type, id, glm::vec3{});
            try {
                entity.deserialize(entityObject);
            } catch (ser::DecodeError& ex) {
                Logger::error(std::format("While deserializing entity (id {}) in '{}': {}", id.id, worldFileName().c_str(), ex.what()));
            }
        }
    } catch (ser::Error& ex) {
        Logger::error(std::format("While deserializing '{}': {}", worldFileName().c_str(), ex.what()));
        return false;
    }

    return true;
}

void World::serializeDeserialize(ser::Object& object) {
    object.field("seed", seed);
    object.field("currentEntityID", currentEntityID.id);
}
