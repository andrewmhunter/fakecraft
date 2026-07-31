#ifndef WORLD_HPP
#define WORLD_HPP

#include <concepts>
#include <map>
#include <memory>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <unordered_map>
#include "chunk.hpp"
#include "entities/entity.hpp"
#include "graphics/graphics.hpp"
#include "level/octree.hpp"
#include "serialization/serialize.hpp"
#include "util/point.hpp"
#include "util/thread_pool.hpp"

class World {
private:
    EntityID currentEntityID;
    CollisionWorld collisionWorld{};

    ThreadPool threadPool;

public:
    std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>, HashIvec3XZOnly> chunks{};
    std::map<EntityID, std::unique_ptr<Entity>> entities{};
    int seed;
    bool showChunkBorders = false;
    int renderDistance;
    Player* player;
    float skyLight = 0.f;
    glm::vec4 skyColor = color::skyblue;

    explicit World();
    World(const World&) = delete;
    World& operator=(const World&) = delete;
    ~World();

    void update(float deltaTime);
    void draw() const;
    Block getBlock(glm::ivec3 worldPoint) const;
    void setBlock(glm::ivec3 worldPoint, Block block);

    Chunk* getChunkRaw(glm::ivec3 chunkCoords);
    const Chunk* getChunkRaw(glm::ivec3 chunkCoords) const;

    Chunk* getChunk(glm::ivec3 chunkCoords, ChunkState atLeastState = ChunkState::loaded);
    const Chunk* getChunk(glm::ivec3 chunkCoords, ChunkState atLeastState = ChunkState::loaded) const;

    void markDirty(glm::ivec3 worldPoint);
    void tryPlaceBlock(glm::ivec3 worldPoint, Block block);
    void tryPlaceBox(glm::ivec3 start, glm::ivec3 size, Block block);
    void placeBox(glm::ivec3 start, glm::ivec3 size, Block block);

    template<std::derived_from<Entity> T, typename... Args>
    T& spawnEntity(Args... args) {
        EntityID id = currentEntityID++;
        std::unique_ptr<T> entity = std::make_unique<T>(this, id, args...);
        T& entityRef = *entity.get();
        entities[id] = std::move(entity);
        return entityRef;
    }

    Entity& spawnEntity(EntityType type, glm::vec3 position);
    Entity& spawnEntity(EntityType type, EntityID id, glm::vec3 position);

    void serialize();
    bool deserialize();
    void serializeDeserialize(ser::Object& object);
};

#endif

