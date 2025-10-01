#include "chunk.h"
#include "mesh.h"
#include "chunk_mesh.h"
#include "logger.h"
#include <stdio.h>

#define MESH_MAX_VERTICIES UINT16_MAX

const Chunk dummyChunk = {0};

void chunkGenerateMesh(Chunk* chunk) {
    ASSERT(chunk);

    const Chunk* adjacentChunks[CHUNK_WIDTH][CHUNK_WIDTH][DIRECTION_CARDINAL_COUNT];

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (Direction dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                adjacentChunks[x][z][dir] = chunk;
            }
        }
    }

    const Chunk* northChunk = chunk->adjacentChunks[DIRECTION_NORTH];
    if (northChunk == NULL) {
        northChunk = &dummyChunk;
    }
    const Chunk* southChunk = chunk->adjacentChunks[DIRECTION_SOUTH];
    if (southChunk == NULL) {
        southChunk = &dummyChunk;
    }
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        adjacentChunks[x][CHUNK_WIDTH - 1][DIRECTION_SOUTH] = southChunk;
        adjacentChunks[x][0][DIRECTION_NORTH] = northChunk;
    }

    const Chunk* eastChunk = chunk->adjacentChunks[DIRECTION_EAST];
    if (eastChunk == NULL) {
        eastChunk = &dummyChunk;
    }
    const Chunk* westChunk = chunk->adjacentChunks[DIRECTION_WEST];
    if (westChunk == NULL) {
        westChunk = &dummyChunk;
    }
    for (int z = 0; z < CHUNK_WIDTH; ++z) {
        adjacentChunks[0][z][DIRECTION_WEST] = westChunk;
        adjacentChunks[CHUNK_WIDTH - 1][z][DIRECTION_EAST] = eastChunk;
    }

    meshListClear(&chunk->meshes);

    chunk->totalVertexCount = 0;

    size_t faceCapacity = 0;
    size_t faceCount = 0;

    Mesh* mesh = NULL;

#ifdef TIME_MESHER
    double timeStart = GetTime();
#endif

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {

#ifdef USE_IGNORED
        if (!chunk->ignored[y]) {
            continue;
        }
#endif

        for (int x = 0; x < CHUNK_WIDTH; ++x) {

#ifdef USE_IGNORED
            if (!(chunk->ignored[y] & 1 << x)) {
                continue;
            }

            bool hasFaces = false;
#endif

            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                Point point = {x, y, z};
                Block block = chunk->blocks[x][y][z];
                const BlockProperties* properties = &blocks[block];

                if (block == BLOCK_AIR) {
                    continue;
                }

                if (faceCount * 4 >= MESH_MAX_VERTICIES - 64 && mesh != NULL) {
                    mesh->vertexCount = faceCount * 4;
                    mesh->triangleCount = faceCount * 2;

                    //printf("%d\n", mesh->vertexCount);
                    UploadMesh(mesh, false);

                    mesh = NULL;
                }

                if (mesh == NULL) {
                    LIST_EXTEND(&chunk->meshes, 1);
                    mesh = &chunk->meshes.data[chunk->meshes.length++];
                    *mesh = (Mesh){0};

                    faceCapacity = 0;
                    faceCount = 0;
                    //}

                    //if (faceCapacity <= (faceCount + 12)) {
                    //faceCapacity = faceCapacity < 256 ? 256 : faceCapacity * 2;
                    faceCapacity = MESH_MAX_VERTICIES / 4;

                    size_t vertexCapacitySize = faceCapacity * 4 * sizeof(float);
                    mesh->vertices = (float*)malloc(vertexCapacitySize * 3);
                    mesh->texcoords = (float*)malloc(vertexCapacitySize * 2);
                    mesh->normals = (float*)malloc(vertexCapacitySize * 3);
                    mesh->colors = (unsigned char*)malloc(faceCapacity * 4 * 4 * sizeof(unsigned char));
                    mesh->indices = (unsigned short*)malloc(faceCapacity * 6 * sizeof(unsigned short));
                }

                const Point* sides = properties->model.sides;

#ifdef USE_IGNORED
#define SET_HAS_FACES() hasFaces = true
#else
#define SET_HAS_FACES()
#endif

                if (properties->solidness == CROSS) {
                    meshCross(mesh, faceCount, x, y, z, properties->model.sides[0].x, properties->model.sides[0].y);
                    faceCount += 4;
                    SET_HAS_FACES();
                    continue;
                }

                for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                    const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                    Point adjacentLocalPoint = worldToLocal(pointAdd(point, directionToPoint(dir)));
                    Block adjacentBlock = chunkGetBlockRaw(adjacentChunk, adjacentLocalPoint); 
                    if (blocks[adjacentBlock].solidness == SOLID || (properties->solidness == TRANSPARENT && block == adjacentBlock)) {
                        continue;
                    }

                    SET_HAS_FACES();

                    meshFaceSmart(mesh, faceCount++, x, y, z, dir,
                            sides[dir].x, sides[dir].y);
                    chunk->totalVertexCount += 4;
                }

                Block adjacentBlock = chunkGetBlockRaw(chunk, pointAddY(point, -1));
                if (y != 0 && blocks[adjacentBlock].solidness != SOLID && (properties->solidness == SOLID || block != adjacentBlock)) {
                    meshFaceSmart(mesh, faceCount++, x, y, z, DIRECTION_DOWN,
                            sides[DIRECTION_DOWN].x, sides[DIRECTION_DOWN].y);
                    chunk->totalVertexCount += 4;

                    SET_HAS_FACES();
                }

                adjacentBlock = chunkGetBlockRaw(chunk, pointAddY(point, 1));
                if ((y == CHUNK_HEIGHT - 1 || blocks[adjacentBlock].solidness != SOLID) && (properties->solidness == SOLID || block != adjacentBlock)) {
                    meshFaceSmart(mesh, faceCount++, x, y, z, DIRECTION_UP,
                            sides[DIRECTION_UP].x, sides[DIRECTION_UP].y);
                    chunk->totalVertexCount += 4;

                    SET_HAS_FACES();
                }

            }

#undef SET_HAS_FACES

#ifdef USE_IGNORED
            if (!hasFaces) {
                chunk->ignored[y] &= ~(1 << x);
            }
#endif
        }
    }

    mesh->triangleCount = faceCount * 2;
    mesh->vertexCount = faceCount * 4;

    //printf("%d\n", mesh->vertexCount);

#ifdef TIME_MESHER
    double buildTime = GetTime() - timeStart;
    timeStart = GetTime();
#endif

    UploadMesh(mesh, false);

#ifdef TIME_MESHER
    double uploadTime = GetTime() - timeStart;
    printf("build %f, upload %f\nratio %f\n", buildTime, uploadTime, buildTime / uploadTime);
#endif


    chunk->dirty = false;
}

