#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "block.hpp"
#include "blocks/BlockModel.hpp"
#include "chunk.hpp"
#include "graphics/mesh.hpp"


std::chrono::duration<double> averageDuration;
int durationCount = 0;

void Chunk::generateMesh() {
    Mesh opaqueMesh{};
    Mesh translucentMesh{};

    auto start = std::chrono::high_resolution_clock::now();

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                glm::ivec3 point = {x, y, z};
                Block block = blocks[x][y][z];

                if (block == Block::air) {
                    continue;
                }

                const BlockInfo& model = getBlockModel(block);
                model.appendGeometry(*this, opaqueMesh, translucentMesh, point, block);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    averageDuration += duration;
    durationCount += 1;
    
    Logger::info(std::format("Time: {}, Average: {}", duration, averageDuration / durationCount));

    cpuMesh = std::move(opaqueMesh);
    cpuTranslucentMesh = std::move(translucentMesh);

    dirty = false;
}

void Chunk::uploadMesh() {
    mesh = cpuMesh->upload();
    cpuMesh = std::nullopt;
    translucentMesh = cpuTranslucentMesh->upload();
    cpuTranslucentMesh = std::nullopt;
}



/*
// This relies on initialization of global variables to all 0s.
const Chunk dummyChunk{};

void Chunk::generateMesh() {
    Mesh opaqueMesh{};
    Mesh translucentMesh{};


    auto start = std::chrono::high_resolution_clock::now();


    const Chunk* adjacentChunks[CHUNK_WIDTH][CHUNK_WIDTH][DIRECTION_CARDINAL_COUNT];

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                adjacentChunks[x][z][dir] = this;
            }
        }
    }

    const Chunk* northChunk = this->adjacentChunks.getChunkDirection(Direction::north);
    if (northChunk == nullptr) {
        northChunk = &dummyChunk;
    }
    const Chunk* southChunk = this->adjacentChunks.getChunkDirection(Direction::south);
    if (southChunk == nullptr) {
        southChunk = &dummyChunk;
    }
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        adjacentChunks[x][CHUNK_WIDTH - 1][Direction::south] = southChunk;
        adjacentChunks[x][0][Direction::north] = northChunk;
    }

    const Chunk* eastChunk = this->adjacentChunks.getChunkDirection(Direction::east);
    if (eastChunk == nullptr) {
        eastChunk = &dummyChunk;
    }
    const Chunk* westChunk = this->adjacentChunks.getChunkDirection(Direction::west);
    if (westChunk == nullptr) {
        westChunk = &dummyChunk;
    }
    for (int z = 0; z < CHUNK_WIDTH; ++z) {
        adjacentChunks[0][z][Direction::west] = westChunk;
        adjacentChunks[CHUNK_WIDTH - 1][z][Direction::east] = eastChunk;
    }


    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                glm::ivec3 point = {x, y, z};
                Block block = blocks[x][y][z];
                const BlockProperties& properties = getBlockProperties(block);

                if (block == Block::air) {
                    continue;
                }

                Mesh& mesh = properties.solidness == Solidness::translucent ? translucentMesh : opaqueMesh;

                const glm::ivec3* sides = properties.model.sides;

                if (properties.solidness == Solidness::cross) {
                    meshCross(mesh, point, properties.model.sides[0]);
                    continue;
                } else if (properties.solidness == Solidness::cactus) {
                    meshCactus(mesh, point, properties.model.sides[0]);
                } else {
                    for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                        const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                        glm::ivec3 adjacentLocalPoint = worldToLocal(point + directionToPoint(static_cast<Direction>(dir)));
                        Block adjacentBlock = adjacentChunk->getBlockRaw(adjacentLocalPoint); 
                        if (getBlockProperties(adjacentBlock).solidness == Solidness::solid || ((properties.solidness == Solidness::transparent || properties.solidness == Solidness::translucent) && block == adjacentBlock)) {
                            continue;
                        }

                        meshFaceSmart(mesh, point, static_cast<Direction>(dir),
                                sides[dir]);
                    }
                }

                Block adjacentBlock = getBlockRaw(point + glm::ivec3{0, -1, 0});
                if (y != 0 && (getBlockProperties(adjacentBlock).solidness != Solidness::solid && (properties.solidness == Solidness::solid || block != adjacentBlock))) {
                    meshFaceSmart(mesh, point, Direction::down,
                            sides[Direction::down]);
                }

                adjacentBlock = getBlockRaw(point + glm::ivec3{0, 1, 0});
                if ((y == CHUNK_HEIGHT - 1 || getBlockProperties(adjacentBlock).solidness != Solidness::solid) && (properties.solidness == Solidness::solid || block != adjacentBlock)) {
                    meshFaceSmart(mesh, point, Direction::up,
                            sides[Direction::up]);
                }

            }
        }
    }


    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    averageDuration += duration;
    durationCount += 1;
    
    Logger::info(std::format("Time: {}, Average: {}", duration, averageDuration / durationCount));



    cpuMesh = std::move(opaqueMesh);
    cpuTranslucentMesh = std::move(translucentMesh);

    dirty = false;
}
*/

