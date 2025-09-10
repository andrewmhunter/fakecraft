#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stdint.h>
#include <m-lib/m-array.h>
#include <raylib.h>
#include "block.h"
#include "config.h"
#include "util.h"

#define CHUNK_SIZE {CHUNK_WIDTH, CHUNK_HEIGHT, CHUNK_WIDTH}

// Positions

static inline Point worldToChunk(Point worldPoint) {
    return (Point){
        floorDiv(worldPoint.x, CHUNK_WIDTH),
        floorDiv(worldPoint.y, CHUNK_HEIGHT),
        floorDiv(worldPoint.z, CHUNK_WIDTH)
    };
}

static inline Point worldToChunkV(Vector3 worldVector) {
    return vector3ToPoint(Vector3Divide(worldVector, (Vector3)CHUNK_SIZE));
    //return worldToChunk(vector3ToPoint(worldPoint));
}

static inline Point worldToLocal(Point worldPoint) {
    Point p = {
        positiveModulo(worldPoint.x, CHUNK_WIDTH),
        positiveModulo(worldPoint.y, CHUNK_HEIGHT),
        positiveModulo(worldPoint.z, CHUNK_WIDTH)
    };

    return p;
}

static inline Point localToWorld(Point chunkCoord, Point local) {
    return pointAdd(pointMultiply(chunkCoord, (Point)CHUNK_SIZE), local);
}

static inline Vector3 worldToLocalV(Vector3 worldVector) {
    Vector3 v = Vector3Subtract(
        worldVector,
        pointToVector3(pointMultiply(worldToChunkV(worldVector), (Point)CHUNK_SIZE))
    );

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

static inline Vector3 localToWorldV(Point chunkCoord, Vector3 local) {
    return Vector3Add(pointToVector3(pointMultiply(chunkCoord, (Point)CHUNK_SIZE)), local);
}


// Chunk

typedef struct {
    Block block;
    uint8_t surfaceHeight;
} BlockInstance;

ARRAY_DEF(array_mesh, Mesh, (CLEAR(UnloadMesh), /*INIT_SET(API_6(meshSet)), SET(API_6(meshSet)),*/ INIT()))

struct World;

typedef struct {
    struct World* world;
    Point coords;
    Block blocks[CHUNK_WIDTH][CHUNK_HEIGHT][CHUNK_WIDTH];
    int surfaceHeight[CHUNK_WIDTH][CHUNK_WIDTH];
    array_mesh_t meshes;
    int totalVertexCount;
#ifdef USE_IGNORED
    uint16_t ignored[CHUNK_HEIGHT];
#endif
    bool dirty;
} Chunk;

static inline Block chunkGetBlockRaw(const Chunk* chunk, Point local) {
    return chunk->blocks[local.x][local.y][local.z];
}

void chunkInit(Chunk* chunk, struct World* world, Point coords);
void chunkUnload(Chunk* chunk);

void chunkPlaceFeatures(Chunk* chunk);
void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block);

bool blockInChunk(const Chunk* chunk, Point local);
void chunkMarkDirty(Chunk* chunk, Point local);
void chunkSetBlockRaw(Chunk* chunk, Point local, Block block);
void chunkSetBlock(Chunk* chunk, Point local, Block block);
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

