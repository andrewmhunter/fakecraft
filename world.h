#ifndef WORLD_H
#define WORLD_H

#include <m-lib/m-array.h>
#include <m-lib/m-dict.h>
#include "chunk.h"
#include "util.h"

ARRAY_DEF(array_chunk, Chunk*, (CLEAR()));
DICT_DEF2(dict_chunk, Point, (HASH(pointHash), EQUAL(pointEquals)), Chunk*, (CLEAR(), INIT()));

#define WORLD_MAX_CHUNK_WIDTH 16

typedef struct World {
    Chunk* chunksArr[WORLD_MAX_CHUNK_WIDTH][WORLD_MAX_CHUNK_WIDTH];
    dict_chunk_t chunks;
    int seed;
    bool showChunkBorders;
} World;

void worldInit(World* world);
void worldUnload(World* world);
void worldDraw(World* world, Material material);
Block worldGetBlock(const World* world, Point worldPoint);
void worldSetBlock(World* world, Point worldPoint, Block block);
Chunk* worldGetChunk(World* world, Point chunkCoords);
const Chunk* worldGetChunkConst(const World* world, Point chunkCoords);

void placeTree(World* world, Point worldPoint);
void placeDungeon(World* world, Point worldPoint);

void worldMarkDirty(World* world, Point worldPoint);
void worldTryPlaceBlock(World* world, Point worldPoint, Block block);
void worldTryPlaceBox(World* world, Point start, Point end, Block block);
void worldPlaceBox(World* world, Point start, Point end, Block block);

typedef struct {
    Vector3 collisionAt;
    Vector3 collisionBefore;
    bool collided;
} Collision;

Collision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength);

extern int shaderModelUniform;

#endif

