#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "block.hpp"
#include "chunk.hpp"
#include "graphics/mesh.hpp"

// This relies on initialization of global variables to all 0s.
const Chunk dummyChunk{};

void Chunk::generateMesh() {
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

    Mesh opaqueMesh{};
    Mesh translucentMesh{};

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
                    meshCross(mesh, x, y, z, properties.model.sides[0].x, properties.model.sides[0].y);
                    continue;
                } else if (properties.solidness == Solidness::cactus) {
                    meshCactus(mesh, x, y, z, properties.model.sides[0].x, properties.model.sides[0].y);
                } else {
                    for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                        const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                        glm::ivec3 adjacentLocalPoint = worldToLocal(point + directionToPoint(static_cast<Direction>(dir)));
                        Block adjacentBlock = adjacentChunk->getBlockRaw(adjacentLocalPoint); 
                        if (getBlockProperties(adjacentBlock).solidness == Solidness::solid || ((properties.solidness == Solidness::transparent || properties.solidness == Solidness::translucent) && block == adjacentBlock)) {
                            continue;
                        }

                        meshFaceSmart(mesh, x, y, z, static_cast<Direction>(dir),
                                sides[dir].x, sides[dir].y);
                    }
                }

                Block adjacentBlock = getBlockRaw(point + glm::ivec3{0, -1, 0});
                if (y != 0 && (getBlockProperties(adjacentBlock).solidness != Solidness::solid && (properties.solidness == Solidness::solid || block != adjacentBlock))) {
                    meshFaceSmart(mesh, x, y, z, Direction::down,
                            sides[Direction::down].x, sides[Direction::down].y);
                }

                adjacentBlock = getBlockRaw(point + glm::ivec3{0, 1, 0});
                if ((y == CHUNK_HEIGHT - 1 || getBlockProperties(adjacentBlock).solidness != Solidness::solid) && (properties.solidness == Solidness::solid || block != adjacentBlock)) {
                    meshFaceSmart(mesh, x, y, z, Direction::up,
                            sides[Direction::up].x, sides[Direction::up].y);
                }

            }
        }
    }

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

