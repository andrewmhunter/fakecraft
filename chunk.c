#include <raylib.h>
#include <raymath.h>
#include "util.h"
#include "chunk.h"
#include "world.h"
#include "chunk.h"
#include "worldgen.h"
#include "serialize.h"

// Chunk

void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block) {
    assert(chunk);

    Point point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

Chunk* chunkInit(World* world, Point coords) {
    assert(world);

    Chunk* chunk = malloc(sizeof(Chunk));
    assert(chunk);

    chunk->world = world;
    chunk->coords = coords;
    array_mesh_init(chunk->meshes);

#ifdef USE_IGNORED
    memset(chunk->ignored, 0xff, sizeof(chunk->ignored));
#endif

    dict_chunk_set_at(world->chunks, coords, chunk);

#ifdef USE_ARRAY
    world->chunksArr[x + WORLD_MAX_CHUNK_WIDTH / 2][z + WORLD_MAX_CHUNK_WIDTH / 2] = chunk;
#endif

    if (loadChunk(chunk)) {
        TraceLog(LOG_WARNING, "Chunk %d, %d loaded from file, %d", chunk->coords.x, chunk->coords.z, pointHash(chunk->coords));
        return chunk;
    }

    generateTerrain(chunk);
    placeFeatures(chunk);

    TraceLog(LOG_WARNING, "Chunk %d, %d generated, %d", chunk->coords.x, chunk->coords.z, pointHash(chunk->coords));
    return chunk;
}


void chunkUnload(Chunk* chunk) {
    assert(chunk);

    saveChunk(chunk);
    TraceLog(LOG_WARNING, "Chunk %d, %d saved, %d", chunk->coords.x, chunk->coords.z, pointHash(chunk->coords));

    array_mesh_clear(chunk->meshes);
    //dict_chunk_erase(chunk->world->chunks, chunk->coords);

#ifdef USE_ARRAY
    world->chunksArr[x + WORLD_MAX_CHUNK_WIDTH / 2][z + WORLD_MAX_CHUNK_WIDTH / 2] = NULL;
#endif

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

