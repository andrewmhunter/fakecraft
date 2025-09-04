#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <m-lib/m-array.h>
#include <raylib.h>
#include "block.h"
#include "util.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 64

#define CHUNK_SIZE {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH}

Point worldToChunk(Point worldPoint);
Point worldToChunkV(Vector3 worldPoint);

Point worldToLocal(Point worldPoint);
Point localToWorld(Point chunkCoord, Point local);

Vector3 worldToLocalV(Vector3 worldVector);
Vector3 localToWorldV(Point chunkCoord, Vector3 local);

static inline void meshSet(Mesh* obj, const Mesh* org) {
    *obj = *org;
}

ARRAY_DEF(array_mesh, Mesh, (CLEAR(UnloadMesh), INIT_SET(API_6(meshSet)), SET(API_6(meshSet)), INIT()));

struct World;

typedef struct {
    struct World* world;
    Point coords;
    char blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    array_mesh_t meshes;
    int totalVertexCount;
    bool dirty;
} Chunk;

void chunkInit(Chunk* chunk, struct World* world, Point coords);

bool blockInChunk(const Chunk* chunk, Point local);
void chunkSetBlock(Chunk* chunk, Point local, Block block);
Block chunkGetBlock(const Chunk* chunk, Point local);
bool chunkIsSolid(const Chunk* chunk, Point local);
bool surroundedBySolid(const Chunk* chunk, Point local);
void chunkGenerateMesh(Chunk* chunk);
void drawChunk(const Chunk* chunk, Material material);

/*typedef struct {
    Vector3 collisionAt;
    Vector3 collisionBefore;
    bool collided;
} Collision;

Collision chunkWalkRay(const Chunk* chunk, Vector3 start, Vector3 direction, float maxLength);*/

#endif

