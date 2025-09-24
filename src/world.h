#ifndef WORLD_H
#define WORLD_H

#include "chunk.h"
#include "entity.h"
#include "mem.h"
#include "hash.h"
#include "point.h"

typedef SPAN_T(Entity*) EntitySpan;
typedef LIST_TS(Entity*, EntitySpan) EntityList;

typedef struct World {
    Set chunks;
    EntityList entities;
    int seed;
    bool showChunkBorders;
    int renderDistance;
    Entity* player;
    float skyLight;
} World;

void worldInit(World* world);
void worldUnload(World* world);
void worldUpdate(World* world, float deltaTime);
void worldDraw(World* world, Material material);
Block worldGetBlock(const World* world, Point worldPoint);
void worldSetBlock(World* world, Point worldPoint, Block block);
Chunk* worldGetChunk(World* world, Point chunkCoords);
const Chunk* worldGetChunkConst(const World* world, Point chunkCoords);

void worldMarkDirty(World* world, Point worldPoint);
void worldTryPlaceBlock(World* world, Point worldPoint, Block block);
void worldTryPlaceBox(World* world, Point start, Point end, Block block);
void worldPlaceBox(World* world, Point start, Point end, Block block);

Entity* spawnEntity(World* world, EntityType type, Vector3 position, float width, float height);

extern int shaderModelUniform;
extern int shaderSkylight;


static inline Hash pointHashWrapper(Hash hash, const void* point) {
    return point2Hash(hash, *((Point*)point));
}

static inline bool pointEqualsWrapper(const void* left, const void* right) {
    return pointEquals(*(Point*)left, *(Point*)right);
}

#define CHUNK_DICT_VTABLE (SetVTable){pointHashWrapper, pointEqualsWrapper}

#endif

