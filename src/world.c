#include <raylib.h>
#include <raymath.h>
#include <stdint.h>
#include "world.h"
#include "util.h"
#include "chunk.h"
#include "chunk_mesh.h"
#include "serialize.h"
#include "logger.h"

void worldTryPlaceBox(World* world, Point start, Point size, Block block) {
    ASSERT(world);

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
    ASSERT(world);

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
    ASSERT(world);

    world->renderDistance = DEFAULT_RENDER_DISTANCE;
    world->showChunkBorders = false;
    world->skyLight = 0.f;
    world->skyColor = SKYBLUE;

    setInit(&world->chunks);
    LIST_INIT(&world->entities);

    Vector3 playerPosition = (Vector3){
        0,
        SURFACE_OFFSET + 10, // worldGetChunk(&world, point(0, 0, 0))->surfaceHeight[0][0] + PLAYER_EYE + 2.f,
        0
    };

    world->player = spawnEntity(world, ENTITY_PLAYER, playerPosition, 0.6f, 1.8f);

    if (loadWorld(world)) {
        return;
    }

#ifdef DEFAULT_SET_SEED
    world->seed = DEFAULT_SET_SEED;
#else
    world->seed = randomInt(10000);
#endif

    /*const int renderDistance = 1;

    for (int x = -renderDistance; x <= renderDistance; ++x) {
        for (int z = -renderDistance; z <= renderDistance; ++z) {
            Point point = {x, 0, z};

            chunkInit(world, point);
        }
    }*/
}

void worldUnload(World* world) {
    saveWorld(world);

    HashEntry* chunkIt = NULL;
    while (setIterate(&world->chunks, &chunkIt)) {
        Chunk* chunk = chunkIt->contents;
        chunkUnload(chunk);
        free(chunk);
    }

    setUnload(&world->chunks);

    for (size_t i = 0; i < world->entities.length; ++i) {
        entityUnload(world->entities.data[i]);
    }
    LIST_FREE(&world->entities);
}

int shaderModelUniform = 0;
int shaderSkylight = 0;
int shaderFogColor = 0;
int shaderFogDistance = 0;
int shaderFogDropoff = 0;

static int chunkDistance(Point from, Point to) {
    return (int)floorf(sqrtf(squaref(from.x - to.x) + squaref(from.z - to.z)));
}


CircleIterator circleIteratorInit(int distance) {
    return (CircleIterator) {
        .distance = distance,
        .row = 0,
        .direction = 4,
        .column = 0,
        .side = 1,
    };
}

static bool updateStateCircleIterator(CircleIterator* state) {
    if (state->side == 1) {
        state->side = -1;
    } else if (state->side == -1) {
        if (state->column != state->row) {
            state->side = 1;
            return true;
        }
    }

    state->direction += 1;
    if (state->direction < DIRECTION_CARDINAL_COUNT) {
        return true;
    }
    state->direction = 0;

    state->column += 1;

    state->side = -1;
    if (state->column < state->row + 1) {
        return true;
    }
    state->column = 0;

    state->row += 1;
    state->side = 0;
    if (state->row < state->distance + 1) {
        return true;
    }

    return false;
}

bool iterateCircleIterator(CircleIterator* state, Point* pointOut) {
    if (!updateStateCircleIterator(state)) {
        return false;
    }

    Point directionPoint = directionToPoint(state->direction);
    Point rightAnglePoint = directionToPoint(directionCardinalRightAngle(state->direction));

    Point point = pointScale(directionPoint, state->row);

    Point offsetColumn = pointScale(rightAnglePoint, state->side * state->column);
    point = pointAdd(point, offsetColumn);

    *pointOut = point;

    //fprintf(stderr, "% d % d\n", point.x, point.z);

    return true;
}

void worldUpdate(World* world, float deltaTime) {
    ASSERT(world);

    static int lightDirection = 1;

    if (world->skyLight > 1.f) {
        lightDirection = -1;
    }

    if (world->skyLight < 0.f) {
        lightDirection = 1;
    }

    world->skyLight += lightDirection * deltaTime * 0.1;
    world->skyLight = 1.f;

    for (size_t i = 0; i < world->entities.length; ++i) {
        entityUpdate(world->entities.data[i], deltaTime);
    }

    int renderDistance = world->renderDistance;

    if (world->player) {
        int maxChunkLoads = 2;

        Point chunkOffset = pointZero();
        CircleIterator offsetIterator = circleIteratorInit(renderDistance);

        do {
            Point chunkCoord = pointAdd(chunkOffset, worldToChunkV(world->player->position));
            chunkCoord.y = 0;
            Chunk* chunk = worldGetChunk(world, chunkCoord);

            if (chunk != NULL) {
                continue;
            }

            int distance = chunkDistance(worldToChunkV(world->player->position), chunkCoord);
            if (distance > renderDistance) {
                continue;
            }

            if (maxChunkLoads <= 0 && distance != 0) {
                continue;
            }
            maxChunkLoads--;

            chunkInit(world, chunkCoord);
        } while (iterateCircleIterator(&offsetIterator, &chunkOffset));
    }

    HashEntry* chunkIt = NULL;
    while (setIterate(&world->chunks, &chunkIt)) {
        Chunk* chunk = chunkIt->contents;

        Point playerChunk = worldToChunkV(world->player->position);

        int distance = chunkDistance(playerChunk, chunk->coords);

        if (distance > renderDistance + 1) {
            chunkUnload(chunk);
            setRemove(&world->chunks, CHUNK_DICT_VTABLE, &chunk->coords);
            free(chunk);
            continue;
        }

        if (chunk->dirty) {
            chunkGenerateMesh(chunk);
        }
    }
}

void worldDraw(World* world, Material material) {
    ASSERT(world);
    ASSERT(IsMaterialValid(material));

    SetShaderValue(material.shader, shaderSkylight, &world->skyLight, SHADER_UNIFORM_FLOAT);


    Color fogColor = world->skyColor;
    Vector4 fogColorVector = {fogColor.r, fogColor.g, fogColor.b, fogColor.a};
    fogColorVector = Vector4Scale(fogColorVector, 1.f / 255.f);
    SetShaderValue(material.shader, shaderFogColor, &fogColorVector, SHADER_UNIFORM_VEC4);

    //float fogDropoff = 4000.f;
    //float fogDistance = 5000.f;
    
    float fogDropoff = (world->renderDistance - 1) * 16;
    fogDropoff *= fogDropoff;
    float fogDistance = fogDropoff * 4.f / 5.f;


    SetShaderValue(material.shader, shaderFogDistance, &fogDistance, SHADER_UNIFORM_FLOAT);
    SetShaderValue(material.shader, shaderFogDropoff, &fogDropoff, SHADER_UNIFORM_FLOAT);

    /*HashEntry* chunkIt = NULL;
    while (setIterate(&world->chunks, &chunkIt)) {
        Chunk* chunk = chunkIt->contents;

        drawChunk(chunk, material);
    }*/

    Point chunkOffset = pointZero();
    CircleIterator offsetIterator = circleIteratorInit(world->renderDistance + 1);
    do {
        Point chunkCoord = pointAdd(chunkOffset, worldToChunkV(world->player->position));
        chunkCoord.y = 0;
        Chunk* chunk = worldGetChunk(world, chunkCoord);
        if (chunk == NULL) {
            continue;
        }
        drawChunk(chunk, material);
    } while (iterateCircleIterator(&offsetIterator, &chunkOffset));

    for (size_t i = 0; i < world->entities.length; ++i) {
        entityDraw(world->entities.data[i]);
    }
}

void worldMarkDirty(World* world, Point worldPoint) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }
    chunkMarkDirty(chunk, worldToLocal(worldPoint));
}

bool chunkCoordsInArray(const World* world, Point chunkCoords) {
    // Hide unused parameter warnings
    (void)world;

    static const int limit = WORLD_MAX_CHUNK_WIDTH / 2;
    return chunkCoords.x >= -limit
        && chunkCoords.x < limit
        && chunkCoords.z >= -limit
        && chunkCoords.z < limit
        && chunkCoords.y == 0;
}

const Chunk* worldGetChunkConst(const World* world, Point chunkCoords) {
    ASSERT(world);

    const Chunk* chunk = setGet(&world->chunks, CHUNK_DICT_VTABLE, &chunkCoords);
    return chunk;
}

Chunk* worldGetChunk(World* world, Point chunkCoords) {
    ASSERT(world);

    Chunk* chunk = setGet(&world->chunks, CHUNK_DICT_VTABLE, &chunkCoords);
    return chunk;
}

Block worldGetBlock(const World* world, Point worldPoint) {
    //ASSERT(world);

    const Chunk* chunk = worldGetChunkConst(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        //return BLOCK_BARRIER;
        return BLOCK_AIR;
    }
    return chunkGetBlockRaw(chunk, worldToLocal(worldPoint));
}

void worldSetBlock(World* world, Point worldPoint, Block block) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        WARN("Block attempted to be placed outside of loaded chunks");
        return;
    }
    chunkSetBlock(chunk, worldToLocal(worldPoint), block);
}

void worldTryPlaceBlock(World* world, Point worldPoint, Block block) {
    ASSERT(world);

    Chunk* chunk = worldGetChunk(world, worldToChunk(worldPoint));
    if (chunk == NULL) {
        return;
    }

    Point local = worldToLocal(worldPoint);
    chunkTryPlaceBlock(chunk, local.x, local.y, local.z, block);
}


Entity* spawnEntity(World* world, EntityType type, Vector3 position, float width, float height) {
    ASSERT(world);

    Entity* entity = malloc(sizeof(Entity));
    ASSERT(entity);

    LIST_PUSH(&world->entities, entity);
    entityInit(entity, type, world, position, width, height);
    return entity;
}

