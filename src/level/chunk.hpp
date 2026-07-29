#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <atomic>
#include <cstddef>
#include <filesystem>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include "block.hpp"
#include "engine/config.hpp"
#include "level/collision.hpp"
#include "serialization/serialize.hpp"
#include "util/util.hpp"
#include "util/point.hpp"
#include "util/direction.hpp"
#include "graphics/graphics.hpp"

constexpr glm::ivec3 chunkSize{CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH};
//#define CHUNK_SIZE {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH}

#define ITERATE_CHUNK(XIDENT, YIDENT, ZIDENT) \
    for (int XIDENT = 0; XIDENT < CHUNK_WIDTH; ++XIDENT) \
        for (int YIDENT = 0; YIDENT < CHUNK_HEIGHT; ++YIDENT) \
            for (int ZIDENT = 0; ZIDENT < CHUNK_WIDTH; ++ZIDENT)

#define ITERATE_CHUNK_YXZ(XIDENT, YIDENT, ZIDENT) \
    for (int YIDENT = 0; YIDENT < CHUNK_HEIGHT; ++YIDENT) \
        for (int XIDENT = 0; XIDENT < CHUNK_WIDTH; ++XIDENT) \
            for (int ZIDENT = 0; ZIDENT < CHUNK_WIDTH; ++ZIDENT)

#define ITERATE_CHUNK_XZY(XIDENT, YIDENT, ZIDENT) \
    for (int XIDENT = 0; XIDENT < CHUNK_WIDTH; ++XIDENT) \
        for (int ZIDENT = 0; ZIDENT < CHUNK_WIDTH; ++ZIDENT) \
            for (int YIDENT = 0; YIDENT < CHUNK_HEIGHT; ++YIDENT)

// Positions

constexpr glm::ivec3 worldToChunk(glm::ivec3 worldPoint) {
    return glm::ivec3{
        floorDiv(worldPoint.x, CHUNK_WIDTH),
        floorDiv(worldPoint.y, CHUNK_HEIGHT),
        floorDiv(worldPoint.z, CHUNK_WIDTH)
    };
}

constexpr glm::ivec3 worldToChunkV(glm::vec3 worldVector) {
    glm::ivec3 chunkPosition = vector3ToPoint(worldVector / glm::vec3{chunkSize});
    chunkPosition.y = 0;
    return chunkPosition;
}

constexpr glm::ivec3 worldToLocal(glm::ivec3 worldPoint) {
    return glm::ivec3{
        positiveModulo(worldPoint.x, CHUNK_WIDTH),
        positiveModulo(worldPoint.y, CHUNK_HEIGHT),
        positiveModulo(worldPoint.z, CHUNK_WIDTH)
    };
}

constexpr glm::ivec3 localToWorld(glm::ivec3 chunkCoord, glm::ivec3 local) {
    return chunkCoord * chunkSize + local;
}

constexpr glm::vec3 worldToLocalV(glm::vec3 worldVector) {
    glm::vec3 v = worldVector
        - glm::vec3{worldToChunkV(worldVector) * chunkSize};

    if (v.x < 0) {
        v.x += CHUNK_WIDTH;
    }
    if (v.y < 0) {
        v.y += CHUNK_HEIGHT;
    }
    if (v.z < 0) {
        v.z += CHUNK_WIDTH;
    }
    return v;
}

constexpr glm::vec3 localToWorldV(glm::ivec3 chunkCoord, glm::vec3 local) {
    return glm::vec3{chunkCoord * chunkSize} + local;
}


class Chunk;

class ChunkCache {
private:
    glm::ivec3 centerPosition;
    std::array<Chunk*, 9> chunks{nullptr};


    constexpr std::size_t getIndexLocal(glm::ivec3 localChunkPosition) const {
        return (localChunkPosition.x + 1) + (localChunkPosition.z + 1) * 3;
    }

    constexpr glm::ivec3 getLocalGlobal(glm::ivec3 globalChunkPosition) const {
        return globalChunkPosition - centerPosition;
    }

    constexpr bool inCacheLocal(glm::ivec3 localChunkPosition) const {
        std::size_t index = getIndexLocal(localChunkPosition);
        // Since index is unsigned we know it's always greater than 0
        return index < chunks.size();
    }

    bool inCacheGlobal(glm::ivec3 globalChunkPosition) const;


public:
    explicit ChunkCache(glm::ivec3 centerPosition, Chunk* centerChunk);
    void setChunk(glm::ivec3 localChunkPosition, Chunk* chunk);

    const Chunk* getChunkLocal(glm::ivec3 localChunkPosition) const;
    Chunk* getChunkLocal(glm::ivec3 localChunkPosition);

    const Chunk* getChunkGlobal(glm::ivec3 globalChunkPosition) const;
    Chunk* getChunkGlobal(glm::ivec3 globalChunkPosition);

    const Chunk* getChunkDirection(Direction direction) const;
    Chunk* getChunkDirection(Direction direction);
    
    Block getBlockRawGlobal(glm::ivec3 globalBlockPosition) const;
};


// Chunk

struct LightValues {
    uint8_t blockLight;
    uint8_t skyLight;
};

struct BlockInstance {
    Block block;
    uint8_t surfaceHeight;
};

class World;
class ChunkCache;

enum class ChunkState {
    unloaded,
    generating,
    terrainGenerated,
    loaded,
};

class Chunk {
private:
    void drawMesh(ShaderProgram& shader, const GPUMesh& mesh) const;
    void serializeDeserialize(ser::Object& object);
    std::filesystem::path getFileName() const;

public:
    World* world;
    glm::ivec3 coords;
    std::optional<GPUMesh> mesh;
    std::optional<GPUMesh> translucentMesh;
    std::atomic_bool dirty;
    ChunkCache adjacentChunks{glm::ivec3{0}, this};
    int surfaceHeight[CHUNK_WIDTH][CHUNK_WIDTH];
    Block blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    LightValues light[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];

    std::atomic<ChunkState> state{ChunkState::unloaded};

    explicit Chunk(World* world, glm::ivec3 coords);
    Chunk();
    ~Chunk();

    Chunk(const Chunk& other) = delete;
    Chunk& operator=(const Chunk& other) = delete;

    inline Block getBlockRaw(glm::ivec3 local) const {
        return blocks[local.x][local.y][local.z];
    }

    void generateOrLoad();
    void fillChunkCache();

    void unload();
    void tryPlaceBlock(glm::ivec3 local, Block block);
    void tryPlaceBlock(int x, int y, int z, Block block);
    void setBlockRaw(glm::ivec3 local, Block block);
    void setBlock(glm::ivec3 local, Block block);
    void markDirty(glm::ivec3 local);
    Block getBlock(glm::ivec3 local) const;
    void draw(ShaderProgram& shader) const;
    void drawTranslucent(ShaderProgram& shader) const;
    bool verify() const;
    void computeLightValues();

    void serialize();
    bool deserialize();
    
    BoundingBox getCullBoundingBox() const;

    static bool blockInChunk(glm::ivec3 local);
};


#endif

