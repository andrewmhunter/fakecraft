#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stdint.h>
#include <m-lib/m-array.h>
#include <raylib.h>
#include "block.h"
#include "util.h"

#define CHUNK_WIDTH 16
#define CHUNK_HEIGHT 128

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

typedef struct {
    Block block;
    uint8_t surfaceHeight;
} BlockInstance;

ARRAY_DEF(array_mesh, Mesh, (CLEAR(UnloadMesh), /*INIT_SET(API_6(meshSet)), SET(API_6(meshSet)),*/ INIT()));

struct World;

typedef struct {
    struct World* world;
    Point coords;
    Block blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    int surfaceHeight[CHUNK_WIDTH][CHUNK_WIDTH];
    array_mesh_t meshes;
    int totalVertexCount;
    uint16_t ignored[CHUNK_HEIGHT];
    bool dirty;
} Chunk;

void chunkInit(Chunk* chunk, struct World* world, Point coords);
void chunkUnload(Chunk* chunk);

void chunkPlaceFeatures(Chunk* chunk);
void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block);

bool blockInChunk(const Chunk* chunk, Point local);
void chunkMarkDirty(Chunk* chunk, Point local);
void chunkSetBlockRaw(Chunk* chunk, Point local, Block block);
void chunkSetBlock(Chunk* chunk, Point local, Block block);
Block chunkGetBlockRaw(const Chunk* chunk, Point local);
Block chunkGetBlock(const Chunk* chunk, Point local);
void chunkGenerateMesh(Chunk* chunk);
void drawChunk(const Chunk* chunk, Material material);

void chunkMarkDirty(Chunk* chunk, Point local);

/*typedef struct {
    Vector3 collisionAt;
    Vector3 collisionBefore;
    bool collided;
} Collision;

Collision chunkWalkRay(const Chunk* chunk, Vector3 start, Vector3 direction, float maxLength);*/

#endif

