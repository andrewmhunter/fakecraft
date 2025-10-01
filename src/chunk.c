#include <raylib.h>
#include <raymath.h>
#include "logger.h"
#include "util.h"
#include "chunk.h"
#include "world.h"
#include "chunk.h"
#include "worldgen.h"
#include "serialize.h"
#include "logger.h"

// Chunk

void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block) {
    ASSERT(chunk);

    Point point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

Chunk* chunkInit(World* world, Point coords) {
    ASSERT(world);

    Chunk* chunk = malloc(sizeof(Chunk));
    ASSERT(chunk);

    chunk->world = world;
    chunk->coords = coords;

    for (Direction i = 0; i < DIRECTION_CARDINAL_COUNT; ++i) {
        Chunk* adjacent = worldGetChunk(world, pointAdd(coords, directionToPoint(i)));
        chunk->adjacentChunks[i] = adjacent;
        if (adjacent != NULL) {
            adjacent->adjacentChunks[invertDirection(i)] = chunk;
            adjacent->dirty = true;
        }
    }

    LIST_INIT(&chunk->meshes);

#ifdef USE_IGNORED
    memset(chunk->ignored, 0xff, sizeof(chunk->ignored));
#endif

    setInsert(&world->chunks, CHUNK_DICT_VTABLE, chunk);


    if (loadChunk(chunk)) {
        DEBUG("Chunk %d, %d loaded from file", chunk->coords.x, chunk->coords.z);
        return chunk;
    }

    generateTerrain(chunk);
    placeFeatures(chunk);

    DEBUG("Chunk %d, %d generated", chunk->coords.x, chunk->coords.z);
    return chunk;
}


void chunkUnload(Chunk* chunk) {
    ASSERT(chunk);


    for (Direction i = 0; i < DIRECTION_CARDINAL_COUNT; ++i) {
        Chunk* adjacent = chunk->adjacentChunks[i];

        if (adjacent != NULL) {
            adjacent->adjacentChunks[invertDirection(i)] = NULL;
        }

        chunk->adjacentChunks[i] = NULL;
    }

    saveChunk(chunk);
    DEBUG("Chunk %d, %d saved", chunk->coords.x, chunk->coords.z);

    meshListClear(&chunk->meshes);
    LIST_FREE(&chunk->meshes);
}


bool blockInChunk(const Chunk* chunk, Point local) {
    ASSERT(chunk);

    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}

void chunkMarkDirty(Chunk* chunk, Point local) {
    ASSERT(chunk);
    ASSERT(blockInChunk(chunk, local));

    chunk->dirty = true;

#ifdef USE_IGNORED
    chunk->ignored[local.y] |= 1 << local.x;
#endif
}

void chunkSetBlockRaw(Chunk* chunk, Point local, Block block) {
    ASSERT(chunk);

    if (!blockInChunk(chunk, local)) {
        WARN("Block placed outside of chunk");
        return;
    }

    chunk->blocks[local.x][local.y][local.z] = block;
    chunkMarkDirty(chunk, local);
}

void chunkSetBlock(Chunk* chunk, Point local, Block block) {
    ASSERT(chunk);

    chunkSetBlockRaw(chunk, local, block);

    Point worldPoint = localToWorld(chunk->coords, local);

    for (int i = 0; i < DIRECTION_COUNT; ++i) {
        worldMarkDirty(chunk->world, pointAdd(worldPoint, directionToPoint(i)));
    }
}

Block chunkGetBlock(const Chunk* chunk, Point local) {
    ASSERT(chunk);

    if (!blockInChunk(chunk, local)) {
        return BLOCK_AIR;
    }
    return chunk->blocks[local.x][local.y][local.z];
}

void drawChunk(const Chunk* chunk, Material material) {
    ASSERT(chunk);
    ASSERT(IsMaterialValid(material));

    Matrix transform = MatrixTranslateV(pointToVector3(pointMultiply(chunk->coords, (Point)CHUNK_SIZE)));

    SetShaderValueMatrix(material.shader, shaderModelUniform, transform);

    for (size_t i = 0; i < chunk->meshes.length; ++i) {
        DrawMesh(chunk->meshes.data[i], material, transform);
    }

    if (chunk->world->showChunkBorders) {
        DrawCubeWiresV(Vector3Multiply(Vector3AddValue(pointToVector3(chunk->coords), 0.5f), (Vector3)CHUNK_SIZE), (Vector3)CHUNK_SIZE, WHITE);
    }
}

bool verifyChunk(const Chunk* chunk) {
    ASSERT(chunk);

    DEBUG("Verifying chunk %d, %d", chunk->coords.x, chunk->coords.z);

    ITERATE_CHUNK(x, y, z) {
        Block block = chunk->blocks[x][y][z];
        if (block >= BLOCK_COUNT) {
            ERROR("Chunk verification failed. Chunk: %d, %d. Block: %d, %d, %d",
                    chunk->coords.x, chunk->coords.z, x, y, z);
            return false;
        }
    }
    return true;
}

