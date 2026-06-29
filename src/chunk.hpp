#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <glm/fwd.hpp>
#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>
#include "block.hpp"
#include "config.hpp"
#include "util.hpp"
#include "point.hpp"
#include "direction.hpp"
#include "graphics.hpp"

#define CHUNK_SIZE {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH}

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

static inline glm::ivec3 worldToChunk(glm::ivec3 worldPoint) {
    return (glm::ivec3){
        floorDiv(worldPoint.x, CHUNK_WIDTH),
        floorDiv(worldPoint.y, CHUNK_HEIGHT),
        floorDiv(worldPoint.z, CHUNK_WIDTH)
    };
}

static inline glm::ivec3 worldToChunkV(glm::vec3 worldVector) {
    glm::ivec3 chunkPosition = vector3ToPoint(worldVector / glm::vec3 CHUNK_SIZE);
    chunkPosition.y = 0;
    return chunkPosition;
    //return worldToChunk(vector3ToPoint(worldPoint));
}

static inline glm::ivec3 worldToLocal(glm::ivec3 worldPoint) {
    glm::ivec3 p = {
        positiveModulo(worldPoint.x, CHUNK_WIDTH),
        positiveModulo(worldPoint.y, CHUNK_HEIGHT),
        positiveModulo(worldPoint.z, CHUNK_WIDTH)
    };

    return p;
}

static inline glm::ivec3 localToWorld(glm::ivec3 chunkCoord, glm::ivec3 local) {
    return chunkCoord * (glm::ivec3)CHUNK_SIZE + local;
}

static inline glm::vec3 worldToLocalV(glm::vec3 worldVector) {
    glm::vec3 v = worldVector
        - glm::vec3{worldToChunkV(worldVector) * (glm::ivec3)CHUNK_SIZE};

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

static inline glm::vec3 localToWorldV(glm::ivec3 chunkCoord, glm::vec3 local) {
    return glm::vec3{chunkCoord * (glm::ivec3)CHUNK_SIZE} + local;
}


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

class Chunk {
private:
    void drawMesh(ShaderProgram& shader, const GPUMesh& mesh) const;

public:
    World* world;
    glm::ivec3 coords;
    std::optional<GPUMesh> mesh;
    std::optional<GPUMesh> translucentMesh;
#ifdef USE_IGNORED
    uint16_t ignored[CHUNK_HEIGHT];
#endif
    bool dirty;
    Chunk* adjacentChunks[DIRECTION_CARDINAL_COUNT];
    int surfaceHeight[CHUNK_WIDTH][CHUNK_WIDTH];
    Block blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    LightValues light[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    bool loaded{false};

    explicit Chunk(World* world, glm::ivec3 coords);
    Chunk();
    ~Chunk();

    Chunk(const Chunk& other) = delete;
    Chunk& operator=(const Chunk& other) = delete;

    inline Block getBlockRaw(glm::ivec3 local) const {
        return blocks[local.x][local.y][local.z];
    }

    void unload();
    void placeFeatures();
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

    static bool blockInChunk(glm::ivec3 local);
};

#endif

