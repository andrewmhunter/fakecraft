#ifndef WORLD_H
#define WORLD_H

#include <m-lib/m-array.h>
#include <m-lib/m-dict.h>
#include "chunk.h"
#include "config.h"
#include "util.h"
#include "entity.h"

//#define USE_ARRAY

ARRAY_DEF(array_chunk, Chunk*, (CLEAR()))
DICT_DEF2(dict_chunk, Point, (HASH(pointHash), EQUAL(pointEquals)), Chunk*, (CLEAR(), INIT()))

ARRAY_DEF(array_entity, Entity*, (CLEAR()))

typedef struct World {
#ifdef USE_ARRAY
    Chunk* chunksArr[WORLD_MAX_CHUNK_WIDTH][WORLD_MAX_CHUNK_WIDTH];
#endif
    dict_chunk_t chunks;
    array_entity_t entities;
    int seed;
    bool showChunkBorders;
    int renderDistance;
    Entity* player;
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

#endif

