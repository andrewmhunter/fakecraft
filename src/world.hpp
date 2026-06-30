#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <memory>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include "chunk.hpp"
#include "config.hpp"
#include "entity.hpp"
#include "graphics.hpp"
#include "point.hpp"

class World {
public:
    std::map<glm::ivec3, std::unique_ptr<Chunk>, CompareIvec3FO> chunks{};
    std::vector<std::unique_ptr<Entity>> entities{};
    int seed;
    bool showChunkBorders = false;
    int renderDistance;
    Player* player;
    float skyLight = 0.f;
    glm::vec4 skyColor = color::skyblue;

    explicit World();
    ~World();

    void update(float deltaTime);
    void draw(ShaderProgram& terrainShader, ShaderProgram& entityShader) const;
    Block getBlock(glm::ivec3 worldPoint) const;
    void setBlock(glm::ivec3 worldPoint, Block block);

    Chunk* getChunk(glm::ivec3 chunkCoords);
    const Chunk* getChunk(glm::ivec3 chunkCoords) const;

    void markDirty(glm::ivec3 worldPoint);
    void tryPlaceBlock(glm::ivec3 worldPoint, Block block);
    void tryPlaceBox(glm::ivec3 start, glm::ivec3 size, Block block);
    void placeBox(glm::ivec3 start, glm::ivec3 size, Block block);

    template<typename T, typename... Args>
    T& spawnEntity(Args... args) {
        std::unique_ptr<T> entity = std::make_unique<T>(this, args...);
        T& entityRef = *entity.get();
        entities.push_back(std::move(entity));
        return entityRef;
    }
};

extern int shaderModelUniform;
extern int shaderSkylight;
extern int shaderFogColor;
extern int shaderFogDistance;
extern int shaderFogDropoff;

typedef struct {
    int distance;
    int row;
    int direction;
    int column;
    int side;
} CircleIterator;

CircleIterator circleIteratorInit(int distance);
bool iterateCircleIterator(CircleIterator* state, glm::ivec3* pointOut);

#endif

