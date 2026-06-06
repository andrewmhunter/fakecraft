#ifndef WORLD_HPP
#define WORLD_HPP

#include <map>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include "chunk.hpp"
#include "entity.hpp"
#include "point.hpp"

typedef struct World {
    std::map<glm::ivec3, Chunk*, CompareIvec3FO> chunks;
    std::vector<Entity*> entities;
    int seed;
    bool showChunkBorders;
    int renderDistance;
    Entity* player;
    float skyLight;
    glm::vec4 skyColor;
} World;

void worldInit(World* world);
void worldUnload(World* world);
void worldUpdate(World* world, float deltaTime);
void worldDraw(World* world, Shader terrainShader, Shader entityShader);
Block worldGetBlock(const World* world, glm::ivec3 worldPoint);
void worldSetBlock(World* world, glm::ivec3 worldPoint, Block block);
Chunk* worldGetChunk(World* world, glm::ivec3 chunkCoords);
const Chunk* worldGetChunkConst(const World* world, glm::ivec3 chunkCoords);

void worldMarkDirty(World* world, glm::ivec3 worldPoint);
void worldTryPlaceBlock(World* world, glm::ivec3 worldPoint, Block block);
void worldTryPlaceBox(World* world, glm::ivec3 start, glm::ivec3 end, Block block);
void worldPlaceBox(World* world, glm::ivec3 start, glm::ivec3 end, Block block);

Entity* spawnEntity(World* world, EntityType type, glm::vec3 position, float width, float height);

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

