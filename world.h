#ifndef WORLD_H
#define WORLD_H

#include <m-lib/m-array.h>
#include <m-lib/m-dict.h>
#include "chunk.h"
#include "util.h"

ARRAY_DEF(array_chunk, Chunk*, (CLEAR()));
DICT_DEF2(dict_chunk, Point, (HASH(pointHash), EQUAL(pointEquals)), Chunk*, (CLEAR(), INIT()));

typedef struct World {
    dict_chunk_t chunks;
} World;

void worldInit(World* world);
void worldDraw(World* world, Material material);
Block worldGetBlock(const World* world, Point worldPoint);
void worldSetBlock(World* world, Point worldPoint, Block block);

typedef struct {
    Vector3 collisionAt;
    Vector3 collisionBefore;
    bool collided;
} Collision;

Collision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength);

#endif

