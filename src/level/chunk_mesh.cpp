#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "block.hpp"
#include "chunk.hpp"
#include "graphics/mesh.hpp"
#include "chunk_mesh.hpp"
#include "engine/logger.hpp"

// This relies on initialization of global variables to all 0s.
const Chunk dummyChunk{};

void chunkGenerateMesh(Chunk* chunk) {
    Logger::assertion(chunk);

    /*FCMesh mesh2{};
    mesh2.pushTexturedPrism(glm::scale(glm::translate(glm::mat4{1.f}, {0.f, 0.5f, 0.f}), chunkSize),
            {{0.f, 0.f}, {1.f, 1.f}});
    chunk->mesh = mesh2.upload();

    std::cout << "Mesh: " << chunk->mesh.vertexArrayObject << "\n";
    chunk->dirty = false;

    return;*/

    const Chunk* adjacentChunks[CHUNK_WIDTH][CHUNK_WIDTH][DIRECTION_CARDINAL_COUNT];

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                adjacentChunks[x][z][dir] = chunk;
            }
        }
    }

    const Chunk* northChunk = chunk->adjacentChunks[Direction::north];
    if (northChunk == NULL) {
        northChunk = &dummyChunk;
    }
    const Chunk* southChunk = chunk->adjacentChunks[Direction::south];
    if (southChunk == NULL) {
        southChunk = &dummyChunk;
    }
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        adjacentChunks[x][CHUNK_WIDTH - 1][Direction::south] = southChunk;
        adjacentChunks[x][0][Direction::north] = northChunk;
    }

    const Chunk* eastChunk = chunk->adjacentChunks[Direction::east];
    if (eastChunk == NULL) {
        eastChunk = &dummyChunk;
    }
    const Chunk* westChunk = chunk->adjacentChunks[Direction::west];
    if (westChunk == NULL) {
        westChunk = &dummyChunk;
    }
    for (int z = 0; z < CHUNK_WIDTH; ++z) {
        adjacentChunks[0][z][Direction::west] = westChunk;
        adjacentChunks[CHUNK_WIDTH - 1][z][Direction::east] = eastChunk;
    }

    Mesh opaqueMesh{};
    Mesh translucentMesh{};

#ifdef TIME_MESHER
    double timeStart = GetTime();
#endif

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                glm::ivec3 point = {x, y, z};
                Block block = chunk->blocks[x][y][z];
                const BlockProperties& properties = getBlock(block);

                if (block == Block::air) {
                    continue;
                }

                Mesh& mesh = properties.solidness == Solidness::translucent ? translucentMesh : opaqueMesh;

                const glm::ivec3* sides = properties.model.sides;

                if (properties.solidness == Solidness::cross) {
                    meshCross(mesh, x, y, z, properties.model.sides[0].x, properties.model.sides[0].y);
                    continue;
                } else if (properties.solidness == Solidness::cactus) {
                    meshCactus(mesh, x, y, z, properties.model.sides[0].x, properties.model.sides[0].y);
                } else {
                    for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                        const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                        glm::ivec3 adjacentLocalPoint = worldToLocal(point + directionToPoint(static_cast<Direction>(dir)));
                        Block adjacentBlock = adjacentChunk->getBlockRaw(adjacentLocalPoint); 
                        if (getBlock(adjacentBlock).solidness == Solidness::solid || ((properties.solidness == Solidness::transparent || properties.solidness == Solidness::translucent) && block == adjacentBlock)) {
                            continue;
                        }

                        meshFaceSmart(mesh, x, y, z, static_cast<Direction>(dir),
                                sides[dir].x, sides[dir].y);
                    }
                }

                

                Block adjacentBlock = chunk->getBlockRaw(point + glm::ivec3{0, -1, 0});
                if (y != 0 && (getBlock(adjacentBlock).solidness != Solidness::solid && (properties.solidness == Solidness::solid || block != adjacentBlock))) {
                    meshFaceSmart(mesh, x, y, z, Direction::down,
                            sides[Direction::down].x, sides[Direction::down].y);
                }

                adjacentBlock = chunk->getBlockRaw(point + glm::ivec3{0, 1, 0});
                if ((y == CHUNK_HEIGHT - 1 || getBlock(adjacentBlock).solidness != Solidness::solid) && (properties.solidness == Solidness::solid || block != adjacentBlock)) {
                    meshFaceSmart(mesh, x, y, z, Direction::up,
                            sides[Direction::up].x, sides[Direction::up].y);
                }

            }
        }
    }

    //printf("%d\n", mesh->vertexCount);

#ifdef TIME_MESHER
    double buildTime = GetTime() - timeStart;
    timeStart = GetTime();
#endif

    chunk->mesh = opaqueMesh.upload();
    chunk->translucentMesh = translucentMesh.upload();

#ifdef TIME_MESHER
    double uploadTime = GetTime() - timeStart;
    printf("build %f, upload %f\nratio %f\n", buildTime, uploadTime, buildTime / uploadTime);
#endif


    chunk->dirty = false;
}

