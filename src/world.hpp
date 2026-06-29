#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <memory>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include "chunk.hpp"
#include "entity.hpp"
#include "point.hpp"

class World {
public:
    std::map<glm::ivec3, Chunk*, CompareIvec3FO> chunks;
    std::vector<std::unique_ptr<Entity>> entities;
    int seed;
    bool showChunkBorders;
    int renderDistance;
    Player* player;
    float skyLight;
    glm::vec4 skyColor;

    template<typename T, typename... Args>
    T& spawnEntity(Args... args) {
        std::unique_ptr<T> entity = std::make_unique<T>(this, args...);
        T& entityRef = *entity.get();
        entities.push_back(std::move(entity));
        return entityRef;
    }
};

void worldInit(World* world);
void worldUnload(World* world);
void worldUpdate(World* world, float deltaTime);
void worldDraw(World* world, ShaderProgram& terrainShader, ShaderProgram& entityShader);
Block worldGetBlock(const World* world, glm::ivec3 worldPoint);
void worldSetBlock(World* world, glm::ivec3 worldPoint, Block block);
Chunk* worldGetChunk(World* world, glm::ivec3 chunkCoords);
const Chunk* worldGetChunkConst(const World* world, glm::ivec3 chunkCoords);

void worldMarkDirty(World* world, glm::ivec3 worldPoint);
void worldTryPlaceBlock(World* world, glm::ivec3 worldPoint, Block block);
void worldTryPlaceBox(World* world, glm::ivec3 start, glm::ivec3 end, Block block);
void worldPlaceBox(World* world, glm::ivec3 start, glm::ivec3 end, Block block);

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

