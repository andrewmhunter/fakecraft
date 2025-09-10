#include <raylib.h>
#include <raymath.h>
#include "util.h"
#include "chunk.h"
#include "world.h"
#include "chunk.h"
#include "worldgen.h"

// Chunk

void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block) {
    assert(chunk);

    Point point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

void chunkInit(Chunk* chunk, World* world, Point coords) {
    assert(chunk);
    assert(world);

    chunk->world = world;
    chunk->coords = coords;
    array_mesh_init(chunk->meshes);

#ifdef USE_IGNORED
    for (int i = 0; i < CHUNK_HEIGHT; ++i) {
        chunk->ignored[i] = UINT16_MAX;
    }
#endif

    generateTerrain(chunk);
}

void chunkUnload(Chunk* chunk) {
    array_mesh_clear(chunk->meshes);
    free(chunk);
}


bool blockInChunk(const Chunk* chunk, Point local) {
    assert(chunk);

    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}

void chunkMarkDirty(Chunk* chunk, Point local) {
    assert(chunk);
    assert(blockInChunk(chunk, local));

    chunk->dirty = true;

#ifdef USE_IGNORED
    chunk->ignored[local.y] |= 1 << local.x;
#endif
}

void chunkSetBlockRaw(Chunk* chunk, Point local, Block block) {
    assert(chunk);

    if (!blockInChunk(chunk, local)) {
        TraceLog(LOG_WARNING, "block placed outside of chunk");
        return;
    }

    chunk->blocks[local.x][local.y][local.z] = block;
    chunkMarkDirty(chunk, local);
}

void chunkSetBlock(Chunk* chunk, Point local, Block block) {
    assert(chunk);

    chunkSetBlockRaw(chunk, local, block);

    Point worldPoint = localToWorld(chunk->coords, local);

    for (int i = 0; i < DIRECTION_COUNT; ++i) {
        worldMarkDirty(chunk->world, pointAdd(worldPoint, directionToPoint(i)));
    }
}

Block chunkGetBlock(const Chunk* chunk, Point local) {
    assert(chunk);

    if (!blockInChunk(chunk, local)) {
        return BLOCK_AIR;
    }
    return chunk->blocks[local.x][local.y][local.z];
}

void drawChunk(const Chunk* chunk, Material material) {
    assert(chunk);
    assert(IsMaterialValid(material));

    Matrix transform = MatrixTranslateV(pointToVector3(pointMultiply(chunk->coords, (Point)CHUNK_SIZE)));

    SetShaderValueMatrix(material.shader, shaderModelUniform, transform);
    array_mesh_it_t it;
    for (array_mesh_it(it, chunk->meshes); !array_mesh_end_p(it); array_mesh_next(it)) {
        const Mesh* mesh = array_mesh_cref(it);
        DrawMesh(*mesh, material, transform);
    }

    if (chunk->world->showChunkBorders) {
        DrawCubeWiresV(Vector3Multiply(Vector3AddValue(pointToVector3(chunk->coords), 0.5f), (Vector3)CHUNK_SIZE), (Vector3)CHUNK_SIZE, WHITE);
    }
}

