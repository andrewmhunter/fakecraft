#include <raylib.h>
#include <raymath.h>
#include "world.h"
#include "util.h"
#include "chunk.h"

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

void placeDungeon(World* world, Point worldPoint) {
    assert(world);

    worldPlaceBox(world, worldPoint, point(10, 6, 10), BLOCK_COBBLESTONE);
    worldPlaceBox(world, pointAddValue(worldPoint, 1, 1, 1), point(8, 4, 8), BLOCK_AIR);
}

void placeTree(World* world, Point worldPoint) {
    assert(world);

    static const Point corners[4] = {
        {1, 0, 1},
        {-1, 0, 1},
        {-1, 0, -1},
        {1, 0, -1},
    };

    int height = randomRange(5, 7);
    worldPlaceBox(world, worldPoint, point(1, height, 1), BLOCK_LOG);

    //worldTryPlaceBox(world, pointAddValue(worldPoint, -2, 2, -2), point(5, 2, 5), BLOCK_LEAVES);

    worldTryPlaceBox(world, pointAddValue(worldPoint, -2, height - 3, -1), point(5, 2, 3), BLOCK_LEAVES);
    worldTryPlaceBox(world, pointAddValue(worldPoint, -1, height - 3, -2), point(3, 2, 5), BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        Point scaledCorner = pointScale(corners[i], 2);
        for (int j = height - 3; j < height - 1; ++j) {
            if (!randomChance(1, 2)) {
                continue;
            }
            worldTryPlaceBlock(world, pointAdd(worldPoint, pointAddY(scaledCorner, j)), BLOCK_LEAVES);
        }
    }

    worldTryPlaceBox(world, pointAddValue(worldPoint, -1, height - 1, 0), point(3, 2, 1), BLOCK_LEAVES);
    worldTryPlaceBox(world, pointAddValue(worldPoint, 0, height - 1, -1), point(1, 2, 3), BLOCK_LEAVES);

    for (int i = 0; i < 4; ++i) {
        if (!randomChance(1, 2)) {
            continue;
        }

        worldTryPlaceBlock(world, pointAdd(worldPoint, pointAddY(corners[i], height - 1)), BLOCK_LEAVES);
    }
}

void worldInit(World* world) {
    assert(world);

    world->seed = randomInt(10000);

    dict_chunk_init(world->chunks);

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
        chunkPlaceFeatures(dict_chunk_ref(it)->value);
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

void worldDraw(World* world, Material material) {
    assert(world);
    assert(IsMaterialValid(material));

    dict_chunk_it_t it;
    for (dict_chunk_it(it, world->chunks); !dict_chunk_end_p(it); dict_chunk_next(it)) {
        Chunk* chunk = dict_chunk_ref(it)->value;

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

Collision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength) {
    assert(world);

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

