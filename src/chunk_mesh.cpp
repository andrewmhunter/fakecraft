#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "block.hpp"
#include "chunk.hpp"
#include "mesh.hpp"
#include "chunk_mesh.hpp"
#include "logger.hpp"

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

    Mesh opaqueMesh{};
    Mesh translucentMesh{};

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
                glm::ivec3 point = {x, y, z};
                Block block = chunk->blocks[x][y][z];
                const BlockProperties& properties = getBlock(block);

                if (block == Block::air) {
                    continue;
                }

                Mesh& mesh = properties.solidness == Solidness::translucent ? translucentMesh : opaqueMesh;

                const glm::ivec3* sides = properties.model.sides;

#ifdef USE_IGNORED
#define SET_HAS_FACES() hasFaces = true
#else
#define SET_HAS_FACES()
#endif

                if (properties.solidness == Solidness::cross) {
                    meshCross(mesh, x, y, z, properties.model.sides[0].x, properties.model.sides[0].y);
                    SET_HAS_FACES();
                    continue;
                }

                for (int dir = 0; dir < DIRECTION_CARDINAL_COUNT; ++dir) {
                    const Chunk* adjacentChunk = adjacentChunks[x][z][dir];
                    glm::ivec3 adjacentLocalPoint = worldToLocal(point + directionToPoint(static_cast<Direction>(dir)));
                    Block adjacentBlock = adjacentChunk->getBlockRaw(adjacentLocalPoint); 
                    if (getBlock(adjacentBlock).solidness == Solidness::solid || ((properties.solidness == Solidness::transparent || properties.solidness == Solidness::translucent) && block == adjacentBlock)) {
                        continue;
                    }

                    SET_HAS_FACES();

                    meshFaceSmart(mesh, x, y, z, static_cast<Direction>(dir),
                            sides[dir].x, sides[dir].y);
                }

                Block adjacentBlock = chunk->getBlockRaw(point + glm::ivec3{0, -1, 0});
                if (y != 0 && (getBlock(adjacentBlock).solidness != Solidness::solid && (properties.solidness == Solidness::solid || block != adjacentBlock))) {
                    meshFaceSmart(mesh, x, y, z, DIRECTION_DOWN,
                            sides[DIRECTION_DOWN].x, sides[DIRECTION_DOWN].y);

                    SET_HAS_FACES();
                }

                adjacentBlock = chunk->getBlockRaw(point + glm::ivec3{0, 1, 0});
                if ((y == CHUNK_HEIGHT - 1 || getBlock(adjacentBlock).solidness != Solidness::solid) && (properties.solidness == Solidness::solid || block != adjacentBlock)) {
                    meshFaceSmart(mesh, x, y, z, DIRECTION_UP,
                            sides[DIRECTION_UP].x, sides[DIRECTION_UP].y);

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

