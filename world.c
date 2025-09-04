#include <raylib.h>
#include <raymath.h>
#include "world.h"
#include "util.h"
#include "chunk.h"


void worldInit(World* world) {
    dict_chunk_init(world->chunks);

    for (int x = -2; x < 2; ++x) {
        for (int z = -2; z < 2; ++z) {
            Point point = {x, 0, z};
            Chunk* chunk = MemAlloc(sizeof(Chunk));
            chunkInit(chunk, world, point);
            dict_chunk_set_at(world->chunks, point, chunk);
        }
    }
}

void worldDraw(World* world, Material material) {
    dict_chunk_it_t it;
    for (dict_chunk_it(it, world->chunks); !dict_chunk_end_p(it); dict_chunk_next(it)) {
        Chunk* chunk = dict_chunk_ref(it)->value;

        if (chunk->dirty) {
            chunkGenerateMesh(chunk);
        }

        drawChunk(chunk, material);
    }
}

Block worldGetBlock(const World* world, Point worldPoint) {
    Chunk* const* chunk = dict_chunk_cget(world->chunks, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return BLOCK_AIR;
    }
    return chunkGetBlock(*chunk, worldToLocal(worldPoint));
}

void worldSetBlock(World* world, Point worldPoint, Block block) {
    Chunk** chunk = dict_chunk_get(world->chunks, worldToChunk(worldPoint));
    if (chunk == NULL) {
        TraceLog(LOG_WARNING, "Block attempted to be placed outside of loaded chunks");
        return;
    }
    chunkSetBlock(*chunk, worldToLocal(worldPoint), block);
}

Collision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength) {
    direction = Vector3Normalize(direction);
    float epsilon = 0.05;
    Vector3 step = Vector3Scale(direction, epsilon);

    Collision coll;
    coll.collided = false;
    coll.collisionBefore = start;
    coll.collisionAt = start;

    Vector3 walk = {0, 0, 0};
    while (Vector3Length(walk) <= maxLength) {
        coll.collisionBefore = coll.collisionAt;
        walk = Vector3Add(walk, step);
        coll.collisionAt = Vector3Add(start, walk);
        if (worldGetBlock(world, vector3ToPoint(coll.collisionAt)) != BLOCK_AIR) {
            coll.collided = true;
            break;
        }
    }

    return coll;
}

