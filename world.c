#include <raylib.h>
#include <raymath.h>
#include "world.h"
#include "util.h"
#include "chunk.h"
#include "chunk_mesh.h"
#include "worldgen.h"

void worldTryPlaceBox(World* world, Point start, Point size, Block block) {
    assert(world);

    Point end = pointAdd(start, size);
    Point realStart = pointMin(start, end);
    Point realEnd = pointMax(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                worldTryPlaceBlock(world, (Point){x, y, z}, block);
            }
        }
    }
}

void worldPlaceBox(World* world, Point start, Point size, Block block) {
    assert(world);

    Point end = pointAdd(start, size);
    Point realStart = pointMin(start, end);
    Point realEnd = pointMax(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                worldSetBlock(world, (Point){x, y, z}, block);
            }
        }
    }
}

void worldInit(World* world) {
    assert(world);

#ifdef DEFAULT_SET_SEED
    world->seed = DEFAULT_SET_SEED;
#else
    world->seed = randomInt(10000);
#endif

    dict_chunk_init(world->chunks);

    array_entity_init(world->entities);

    for (int i = 0; i < WORLD_MAX_CHUNK_WIDTH; ++i) {
        for (int j = 0; j < WORLD_MAX_CHUNK_WIDTH; ++j) {
            world->chunksArr[i][j] = NULL;
        }
    }

    const int renderDistance = 5;

    for (int x = -renderDistance; x <= renderDistance; ++x) {
        for (int z = -renderDistance; z <= renderDistance; ++z) {
            Point point = {x, 0, z};
            Chunk* chunk = malloc(sizeof(Chunk));
            assert(chunk);

            chunkInit(chunk, world, point);

            world->chunksArr[x + WORLD_MAX_CHUNK_WIDTH / 2][z + WORLD_MAX_CHUNK_WIDTH / 2] = chunk;
            dict_chunk_set_at(world->chunks, point, chunk);
        }
    }

    dict_chunk_it_t it;
    for (dict_chunk_it(it, world->chunks); !dict_chunk_end_p(it); dict_chunk_next(it)) {
        placeFeatures(dict_chunk_ref(it)->value);
    }

    world->showChunkBorders = false;
}

void worldUnload(World* world) {
    dict_chunk_it_t it;
    for (dict_chunk_it(it, world->chunks); !dict_chunk_end_p(it); dict_chunk_next(it)) {
        Chunk* chunk = dict_chunk_ref(it)->value;
        chunkUnload(chunk);
    }
    dict_chunk_clear(world->chunks);
}

int shaderModelUniform = 0;

void worldUpdate(World* world, float deltaTime) {
    assert(world);

    array_entity_it_t entityIt;
    for (array_entity_it(entityIt, world->entities); !array_entity_end_p(entityIt); array_entity_next(entityIt)) {
        entityUpdate(*array_entity_cref(entityIt), deltaTime);
    }
}

void worldDraw(World* world, Material material) {
    assert(world);
    assert(IsMaterialValid(material));

    array_entity_it_t entityIt;
    for (array_entity_it(entityIt, world->entities); !array_entity_end_p(entityIt); array_entity_next(entityIt)) {
        entityDraw(*array_entity_cref(entityIt));
    }

    dict_chunk_it_t chunkIt;
    for (dict_chunk_it(chunkIt, world->chunks); !dict_chunk_end_p(chunkIt); dict_chunk_next(chunkIt)) {
        Chunk* chunk = dict_chunk_ref(chunkIt)->value;

        if (chunk->dirty) {
            chunkGenerateMesh(chunk);
        }

        drawChunk(chunk, material);
    }
}

void worldMarkDirty(World* world, Point worldPoint) {
    assert(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }
    chunkMarkDirty(chunk, worldToLocal(worldPoint));
}

bool chunkCoordsInArray(const World* world, Point chunkCoords) {
    static const int limit = WORLD_MAX_CHUNK_WIDTH / 2;
    return chunkCoords.x >= -limit
        && chunkCoords.x < limit
        && chunkCoords.z >= -limit
        && chunkCoords.z < limit
        && chunkCoords.y == 0;
}

const Chunk* worldGetChunkConst(const World* world, Point chunkCoords) {
    assert(world);

    if (!chunkCoordsInArray(world, chunkCoords)) {
        return NULL;
    }

    const Chunk* chunk = world->chunksArr[chunkCoords.x + WORLD_MAX_CHUNK_WIDTH / 2][chunkCoords.z + WORLD_MAX_CHUNK_WIDTH / 2];
    return chunk;
}

Chunk* worldGetChunk(World* world, Point chunkCoords) {
    assert(world);

    if (!chunkCoordsInArray(world, chunkCoords)) {
        return NULL;
    }

    Chunk* chunk = world->chunksArr[chunkCoords.x + WORLD_MAX_CHUNK_WIDTH / 2][chunkCoords.z + WORLD_MAX_CHUNK_WIDTH / 2];
    return chunk;
}

Block worldGetBlock(const World* world, Point worldPoint) {
    //assert(world);

    const Chunk* chunk = worldGetChunkConst(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        //return BLOCK_BARRIER;
        return BLOCK_AIR;
    }
    return chunkGetBlockRaw(chunk, worldToLocal(worldPoint));
}

void worldSetBlock(World* world, Point worldPoint, Block block) {
    assert(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        TraceLog(LOG_WARNING, "Block attempted to be placed outside of loaded chunks");
        return;
    }
    chunkSetBlock(chunk, worldToLocal(worldPoint), block);
}

void worldTryPlaceBlock(World* world, Point worldPoint, Block block) {
    assert(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }

    Point local = worldToLocal(worldPoint);
    chunkTryPlaceBlock(chunk, local.x, local.y, local.z, block);
}


Entity* spawnEntity(World* world, EntityType type, Vector3 position, float width, float height) {
    assert(world);

    Entity* entity = malloc(sizeof(Entity));
    array_entity_push_move(world->entities, &entity);
    entityInit(entity, type, world, position, width, height);
    return entity;
}

