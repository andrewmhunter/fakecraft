#include <raylib.h>
#include <raymath.h>
#include "mesh.h"
#include "util.h"
#include "chunk.h"
#include "world.h"

// Positions

Point worldToChunk(Point worldPoint) {
    return (Point){
        floorDiv(worldPoint.x, CHUNK_WIDTH),
        floorDiv(worldPoint.y, CHUNK_HEIGHT),
        floorDiv(worldPoint.z, CHUNK_WIDTH)
    };
}

Point worldToChunkV(Vector3 worldVector) {
    return vector3ToPoint(Vector3Divide(worldVector, (Vector3)CHUNK_SIZE));
    //return worldToChunk(vector3ToPoint(worldPoint));
}

Point worldToLocal(Point worldPoint) {
    Point p = {
        positiveModulo(worldPoint.x, CHUNK_WIDTH),
        positiveModulo(worldPoint.y, CHUNK_HEIGHT),
        positiveModulo(worldPoint.z, CHUNK_WIDTH)
    };

    return p;
}

Point localToWorld(Point chunkCoord, Point local) {
    return pointAdd(pointMultiply(chunkCoord, (Point)CHUNK_SIZE), local);
}

Vector3 worldToLocalV(Vector3 worldVector) {
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

Vector3 localToWorldV(Point chunkCoord, Vector3 local) {
    return Vector3Add(pointToVector3(pointMultiply(chunkCoord, (Point)CHUNK_SIZE)), local);
}

// Chunk

void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block) {
    assert(chunk);

    Point point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

//#define SURFACE 20
#define DIRT_LAYER 1

void chunkInit(Chunk* chunk, World* world, Point coords) {
    assert(chunk);
    assert(world);

    chunk->world = world;
    chunk->coords = coords;

    for (int i = 0; i < CHUNK_HEIGHT; ++i) {
        chunk->ignored[i] = UINT16_MAX;
    }

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                chunk->blocks[x][y][z] = BLOCK_AIR;
            }
        }
    }

    array_mesh_init(chunk->meshes);

    Image noise0 = GenImagePerlinNoise(16, 16, 16 * coords.x + world->seed, 16 * coords.z, 0.06125);
    Image noise1 = GenImagePerlinNoise(16, 16, 16 * coords.x + world->seed, 16 * coords.z, 0.125);
    Image noise2 = GenImagePerlinNoise(16, 16, 16 * coords.x + world->seed, 16 * coords.z, 0.25);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            Color c0 = GetImageColor(noise0, x, z);
            Color c1 = GetImageColor(noise1, x, z);
            Color c2 = GetImageColor(noise2, x, z);

            int surface = c0.r / 4 + c1.r / 8 + c2.r / 16;
            //int surface = /*c0.r / 4 +*/ c1.r / 16 + c2.r / 8;

            surface = surface >= CHUNK_HEIGHT ? CHUNK_HEIGHT - 1 : surface;
            chunk->surfaceHeight[x][z] = surface;

            chunk->blocks[x][0][z] = BLOCK_BEDROCK;

            for (int y = 1; y < surface; ++y) {
                Block block = BLOCK_DIRT;

                if (y < surface - DIRT_LAYER) {
                    block = randomChance(1, 100) ? BLOCK_DIAMOND_ORE : BLOCK_STONE;
                }
                chunk->blocks[x][y][z] = block;
            }
            chunk->blocks[x][surface][z] = BLOCK_GRASS;
        }
    }

    chunk->dirty = true;

    UnloadImage(noise0);
    UnloadImage(noise1);
    UnloadImage(noise2);
}

void chunkUnload(Chunk* chunk) {
    array_mesh_clear(chunk->meshes);
    free(chunk);
}

void chunkPlaceFeatures(Chunk* chunk) {
    int treeCount = randomInt(5);
    int SURFACE = 20;
    for (int i = 0; i < treeCount; ++i) {
        Point point = {randomInt(CHUNK_WIDTH), 0, randomInt(CHUNK_WIDTH)};
        point.y = chunk->surfaceHeight[point.x][point.z] + 1;

        placeTree(chunk->world, localToWorld(chunk->coords, point));
    }

    if (randomChance(1, 20)) {
        placeDungeon(chunk->world, localToWorld(chunk->coords, point(randomInt(CHUNK_WIDTH), 10, randomInt(CHUNK_WIDTH))));
    }
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
    chunk->ignored[local.y] |= 1 << local.x;
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

    static const Point xAdjacent = {1, 0, 0};
    static const Point zAdjacent = {0, 0, 1};
    chunkSetBlockRaw(chunk, local, block);

    Point worldPoint = localToWorld(chunk->coords, local);

    for (int i = 0; i < DIRECTION_COUNT; ++i) {
        worldMarkDirty(chunk->world, pointAdd(worldPoint, directionToPoint(i)));
    }

    /*Chunk* adjacent = NULL;
    if (local.x == 0) {
        adjacent = worldGetChunk(chunk->world, pointSubtract(chunk->coords, xAdjacent));
    } else if (local.x == CHUNK_WIDTH - 1) {
        adjacent = worldGetChunk(chunk->world, pointAdd(chunk->coords, xAdjacent));
    }

    if (adjacent != NULL) {
        adjacent->dirty = true;
    }

    if (local.z == 0) {
        adjacent = worldGetChunk(chunk->world, pointSubtract(chunk->coords, zAdjacent));
    } else if (local.z == CHUNK_WIDTH - 1) {
        adjacent = worldGetChunk(chunk->world, pointAdd(chunk->coords, zAdjacent));
    }

    if (adjacent != NULL) {
        adjacent->dirty = true;
    }*/
}


Block chunkGetBlockRaw(const Chunk* chunk, Point local) {
    return chunk->blocks[local.x][local.y][local.z];
}

Block chunkGetBlock(const Chunk* chunk, Point local) {
    assert(chunk);

    if (!blockInChunk(chunk, local)) {
        return BLOCK_AIR;
    }
    return chunk->blocks[local.x][local.y][local.z];
}

const size_t meshMaxVerticies = UINT16_MAX;

Chunk dummyChunk = {0};

void chunkGenerateMesh(Chunk* chunk) {
    assert(chunk);

    const Chunk* adjacentChunks[CHUNK_WIDTH][CHUNK_WIDTH][DIRECTION_CARDINAL_COUNT];

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (Direction dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                adjacentChunks[x][z][dir] = chunk;
            }
        }
    }

    const Chunk* northChunk = worldGetChunkConst(chunk->world,
            pointAdd(chunk->coords, directionToPoint(DIRECTION_NORTH)));
    if (northChunk == NULL) {
        northChunk = &dummyChunk;
    }
    const Chunk* southChunk = worldGetChunkConst(chunk->world,
            pointAdd(chunk->coords, directionToPoint(DIRECTION_SOUTH)));
    if (southChunk == NULL) {
        southChunk = &dummyChunk;
    }
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        adjacentChunks[x][CHUNK_WIDTH - 1][DIRECTION_SOUTH] = southChunk;
        adjacentChunks[x][0][DIRECTION_NORTH] = northChunk;
    }

    const Chunk* eastChunk = worldGetChunkConst(chunk->world,
            pointAdd(chunk->coords, directionToPoint(DIRECTION_EAST)));
    if (eastChunk == NULL) {
        eastChunk = &dummyChunk;
    }
    const Chunk* westChunk = worldGetChunkConst(chunk->world,
            pointAdd(chunk->coords, directionToPoint(DIRECTION_WEST)));
    if (westChunk == NULL) {
        westChunk = &dummyChunk;
    }
    for (int z = 0; z < CHUNK_WIDTH; ++z) {
        adjacentChunks[0][z][DIRECTION_WEST] = westChunk;
        adjacentChunks[CHUNK_WIDTH - 1][z][DIRECTION_EAST] = eastChunk;
    }

    array_mesh_reset(chunk->meshes);

    chunk->totalVertexCount = 0;

    size_t faceCapacity = 0;
    size_t faceCount = 0;

    Mesh* mesh = NULL;

    double timeStart = GetTime();

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        if (!chunk->ignored[y]) {
            continue;
        }

        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            if (!(chunk->ignored[y] & 1 << x)) {
                continue;
            }

            // TODO: Change back to false
            bool hasFaces = false;

            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                Point point = {x, y, z};
                Block block = chunk->blocks[x][y][z];
                const BlockProperties* properties = &blocks[block];

                if (block == BLOCK_AIR) {
                    continue;
                }

                /*if (surroundedBySolid(chunk, point)) {
                    continue;
                }*/

                if (faceCount * 4 >= meshMaxVerticies - 64 && mesh != NULL) {
                    mesh->vertexCount = faceCount * 4;
                    mesh->triangleCount = faceCount * 2;

                    UploadMesh(mesh, false);

                    mesh = NULL;
                }

                if (mesh == NULL) {
                    mesh = array_mesh_push_new(chunk->meshes);
                    *mesh = (Mesh){0};

                    faceCapacity = 0;
                    faceCount = 0;
                    //}

                    //if (faceCapacity <= (faceCount + 12)) {
                    //faceCapacity = faceCapacity < 256 ? 256 : faceCapacity * 2;
                    faceCapacity = meshMaxVerticies / 4;

                    size_t vertexCapacitySize = faceCapacity * 4 * sizeof(float);
                    mesh->vertices = (float*)malloc(vertexCapacitySize * 3);
                    mesh->texcoords = (float*)malloc(vertexCapacitySize * 2);
                    mesh->normals = (float*)malloc(vertexCapacitySize * 3);
                    mesh->indices = (unsigned short*)malloc(faceCapacity * 6 * sizeof(unsigned short));
                }

                const Point* sides = properties->model.sides;

                #if 0
                for (int dir = 0; dir < DIRECTION_COUNT; ++dir) {
                    //const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                    //const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                    //Point adjacentLocalPoint = worldToLocal(pointAdd(point, directionToPoint(dir)));
                    //Block adjacentBlock = chunkGetBlockRaw(adjacentChunk, adjacentLocalPoint); 
                    /*Block adjacentBlock = worldGetBlock(chunk->world, localToWorld(chunk->coords, point)); 
                    if (blocks[adjacentBlock].solidness == SOLID) {
                        continue;
                    }*/

                    hasFaces = true;

                    meshFaceSmart(mesh, faceCount++, x, y, z, dir,
                            sides[dir].x, sides[dir].y);
                    chunk->totalVertexCount += 4;
                }
                #endif


                for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                    const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                    Point adjacentLocalPoint = worldToLocal(pointAdd(point, directionToPoint(dir)));
                    Block adjacentBlock = chunkGetBlockRaw(adjacentChunk, adjacentLocalPoint); 
                    if (blocks[adjacentBlock].solidness == SOLID) {
                        continue;
                    }

                    hasFaces = true;

                    meshFaceSmart(mesh, faceCount++, x, y, z, dir,
                            sides[dir].x, sides[dir].y);
                    chunk->totalVertexCount += 4;
                }

                if (y == 0 || blocks[chunkGetBlockRaw(chunk, pointAddY(point, -1))].solidness != SOLID) {
                    meshFaceSmart(mesh, faceCount++, x, y, z, DIRECTION_DOWN,
                            sides[DIRECTION_DOWN].x, sides[DIRECTION_DOWN].y);
                    chunk->totalVertexCount += 4;

                    hasFaces = true;
                }

                if (y == CHUNK_HEIGHT - 1 || blocks[chunkGetBlockRaw(chunk, pointAddY(point, 1))].solidness != SOLID) {
                    meshFaceSmart(mesh, faceCount++, x, y, z, DIRECTION_UP,
                            sides[DIRECTION_UP].x, sides[DIRECTION_UP].y);
                    chunk->totalVertexCount += 4;

                    hasFaces = true;
                }

            }

            if (!hasFaces) {
                chunk->ignored[y] &= ~(1 << x);
            }
        }
    }

    double buildTime = GetTime() - timeStart;

    mesh->triangleCount = faceCount * 2;
    mesh->vertexCount = faceCount * 4;

    timeStart = GetTime();
    UploadMesh(mesh, false);
    double uploadTime = GetTime() - timeStart;

    printf("build %f, upload %f\nratio %f\n", buildTime, uploadTime, buildTime / uploadTime);

    chunk->dirty = false;
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

