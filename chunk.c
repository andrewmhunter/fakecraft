#include <raylib.h>
#include <raymath.h>
#include "mesh.h"
#include "util.h"
#include "chunk.h"
#include "world.h"

// Positions

Point worldToChunk(Point worldPoint) {
    /*if (worldPoint.x < 0) {
        worldPoint.x -= CHUNK_WIDTH;
    }
    if (worldPoint.y < 0) {
        worldPoint.y -= CHUNK_HEIGHT;
    }
    if (worldPoint.z < 0) {
        worldPoint.z -= CHUNK_WIDTH;
    }

    return pointDivide(worldPoint, (Point)CHUNK_SIZE);*/
    // TODO: There is definitly a faster way to do this than converting to floating points

    return worldToChunkV(pointToVector3(worldPoint));
}

Point worldToChunkV(Vector3 worldVector) {
    return vector3ToPoint(Vector3Divide(worldVector, (Vector3)CHUNK_SIZE));
    //return worldToChunk(vector3ToPoint(worldPoint));
}

Point worldToLocal(Point worldPoint) {
    Point p = (Point){worldPoint.x % CHUNK_WIDTH, worldPoint.y % CHUNK_HEIGHT, worldPoint.z % CHUNK_WIDTH};

    if (p.x < 0) {
        p.x += CHUNK_WIDTH;
    }
    if (p.y < 0) {
        p.y += CHUNK_HEIGHT;
    }
    if (p.z < 0) {
        p.z += CHUNK_WIDTH;
    }
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
    Point point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

void placeTree(Chunk* chunk, int x, int y, int z) {
    for (int i = 0; i < 5; ++i) {
        chunkSetBlock(chunk, (Point){x, y + i, z}, BLOCK_LOG);
    }

    for (int i = 2; i < 4; ++i) {
        for (int j = -2; j <= 2; ++j) {
            for (int k = -2; k <= 2; ++k) {
                chunkTryPlaceBlock(chunk, x + j, y + i, z + k, BLOCK_LEAVES);
            }
        }
    }

    for (int i = 4; i < 6; ++i) {
        chunkTryPlaceBlock(chunk, x - 1, y + i, z, BLOCK_LEAVES);
        chunkTryPlaceBlock(chunk, x + 1, y + i, z, BLOCK_LEAVES);
        chunkTryPlaceBlock(chunk, x, y + i, z - 1, BLOCK_LEAVES);
        chunkTryPlaceBlock(chunk, x, y + i, z + 1, BLOCK_LEAVES);
    }
    chunkTryPlaceBlock(chunk, x, y + 5, z, BLOCK_LEAVES);
}

void chunkInit(Chunk* chunk, World* world, Point coords) {
    chunk->world = world;
    chunk->coords = coords;

    array_mesh_init(chunk->meshes);

    const int surface = 20;
    const int dirtLayer = 3;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            chunk->blocks[x][0][z] = BLOCK_BEDROCK;

            for (int y = 1; y < surface; ++y) {
                Block block = BLOCK_DIRT;

                if (y < surface - dirtLayer - 1) {
                    block = randomChance(1, 100) ? BLOCK_DIAMOND_ORE : BLOCK_STONE;
                }
                chunk->blocks[x][y][z] = block;
            }
            chunk->blocks[x][surface][z] = BLOCK_GRASS;
        }
    }

    /*for (int i = 0; i < 1000; ++i) {
        chunk->blocks[rand() % CHUNK_WIDTH][rand() % CHUNK_HEIGHT][rand() % CHUNK_WIDTH] = rand() % BLOCK_COUNT;
    }*/

    int treeCount = randomInt(5);
    for (int i = 0; i < treeCount; ++i) {
        placeTree(chunk, randomInt(CHUNK_WIDTH), surface + 1, randomInt(CHUNK_WIDTH));
    }

    //chunkGenerateMesh(chunk);
    chunk->dirty = true;
}

bool blockInChunk(const Chunk* chunk, Point local) {
    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}

void chunkSetBlock(Chunk* chunk, Point local, Block block) {
    if (!blockInChunk(chunk, local)) {
        TraceLog(LOG_WARNING, "block placed outside of chunk");
        return;
    }

    chunk->dirty = true;
    chunk->blocks[local.x][local.y][local.z] = block;
}


Block chunkGetBlock(const Chunk* chunk, Point local) {
    if (!blockInChunk(chunk, local)) {
        return BLOCK_AIR;
    }
    return chunk->blocks[local.x][local.y][local.z];
}


bool chunkIsSolid(const Chunk* chunk, Point local) {
    return blocks[chunkGetBlock(chunk, local)].solidness == SOLID;
}

bool surroundedBySolid(const Chunk* chunk, Point local) {
    for (Direction dir = 0; dir < DIRECTION_COUNT; ++dir) {
        if (!chunkIsSolid(chunk, pointAdd(local, directionToPoint(dir)))) {
            return false;
        }
    }
    return true;
    /*return chunkIsSolid(chunk, x + 1, y, z)
        && chunkIsSolid(chunk, x - 1, y, z)
        && chunkIsSolid(chunk, x, y + 1, z)
        && chunkIsSolid(chunk, x, y - 1, z)
        && chunkIsSolid(chunk, x, y, z + 1)
        && chunkIsSolid(chunk, x, y, z - 1);*/
}

const int placeCount = 1000;
const size_t meshMaxVerticies = 65536;

void chunkGenerateMesh(Chunk* chunk) {
    array_mesh_reset(chunk->meshes);

    chunk->totalVertexCount = 0;

    size_t faceCapacity = 0;
    size_t faceCount = 0;
    //int cubeCount = 0;

    Mesh* mesh = NULL;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
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
                    //cubeCount = 0;
                    //}

                    //if (faceCapacity <= (faceCount + 12)) {
                    //faceCapacity = faceCapacity < 256 ? 256 : faceCapacity * 2;
                    faceCapacity = meshMaxVerticies / 4;

                    size_t vertexCapacitySize = faceCapacity * 4 * sizeof(float);
                    mesh->vertices = (float*)MemRealloc(mesh->vertices, vertexCapacitySize * 3);
                    mesh->texcoords = (float*)MemRealloc(mesh->texcoords, vertexCapacitySize * 2);
                    mesh->normals = (float*)MemRealloc(mesh->normals, vertexCapacitySize * 3);
                    mesh->indices = (unsigned short*)MemRealloc(mesh->indices, faceCapacity * 6 * sizeof(unsigned short));
                }

                for (int dir = 0; dir < DIRECTION_COUNT; ++dir) {
                    if (blocks[worldGetBlock(chunk->world, pointAdd(localToWorld(chunk->coords, point), directionToPoint(dir)))].solidness == SOLID) {
                        continue;
                    }

                    //if (chunkIsSolid(chunk, pointAdd(point, directionToPoint(dir)))) {
                    //    continue;
                    //}

                    meshFaceSmart(mesh, faceCount++, x, y, z, dir, properties->texCoordX, properties->texCoordY);
                    //faceCount += 6;
                    chunk->totalVertexCount += 4;
                }
                //meshAddCube(mesh, faceCount / 6, x, y, z, properties->texCoordX, properties->texCoordY);
                //faceCount += 6;
                //chunk->totalVertexCount += 8;
            }
        }
    }

    mesh->triangleCount = faceCount * 2;
    mesh->vertexCount = faceCount * 4;

    UploadMesh(mesh, false);

    chunk->dirty = false;
}

void drawChunk(const Chunk* chunk, Material material) {
        array_mesh_it_t it;
        for (array_mesh_it(it, chunk->meshes); !array_mesh_end_p(it); array_mesh_next(it)) {
            const Mesh* mesh = array_mesh_cref(it);
            DrawMesh(*mesh, material, MatrixTranslateV(pointToVector3(pointMultiply(chunk->coords, (Point)CHUNK_SIZE))));
        }

        
        DrawCubeWiresV(Vector3Multiply(Vector3AddValue(pointToVector3(chunk->coords), 0.5f), (Vector3)CHUNK_SIZE), (Vector3)CHUNK_SIZE, WHITE);
}

