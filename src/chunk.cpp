#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "logger.hpp"
#include "chunk.hpp"
#include "world.hpp"
#include "chunk.hpp"
#include "worldgen.hpp"
#include "serialize.hpp"
#include "logger.hpp"
#include "graphics.hpp"

// Chunk

void chunkTryPlaceBlock(Chunk* chunk, int x, int y, int z, Block block) {
    Logger::Logger::assertion(chunk);

    glm::ivec3 point = {x, y, z};
    if (chunkGetBlock(chunk, point) != BLOCK_AIR) {
        return;
    }

    chunkSetBlock(chunk, point, block);
}

Chunk* chunkInit(World* world, glm::ivec3 coords) {
    Logger::assertion(world);

    Chunk* chunk = new Chunk;
    Logger::assertion(chunk);

    chunk->world = world;
    chunk->coords = coords;
    chunk->loaded = true;

    for (int i = 0; i < DIRECTION_CARDINAL_COUNT; i += 1) {
        Chunk* adjacent = worldGetChunk(world, coords + directionToPoint(static_cast<Direction>(i)));
        chunk->adjacentChunks[i] = adjacent;
        if (adjacent != NULL) {
            adjacent->adjacentChunks[invertDirection(static_cast<Direction>(i))] = chunk;
            adjacent->dirty = true;
        }
    }

#ifdef USE_IGNORED
    memset(chunk->ignored, 0xff, sizeof(chunk->ignored));
#endif

    world->chunks[coords] = chunk;

    if (loadChunk(chunk)) {
        Logger::debug(std::format("Chunk {}, {} loaded from file", chunk->coords.x, chunk->coords.z));
        return chunk;
    }

    generateTerrain(chunk);
    placeFeatures(chunk);

    Logger::debug(std::format("Chunk {}, {} generated", chunk->coords.x, chunk->coords.z));
    return chunk;
}


void chunkUnload(Chunk* chunk) {
    Logger::assertion(chunk);
    chunk->loaded = false;

    for (int i = 0; i < DIRECTION_CARDINAL_COUNT; ++i) {
        Chunk* adjacent = chunk->adjacentChunks[i];

        if (adjacent != NULL) {
            adjacent->adjacentChunks[invertDirection(static_cast<Direction>(i))] = NULL;
        }

        chunk->adjacentChunks[i] = NULL;
    }

    saveChunk(chunk);
    Logger::debug(std::format("Chunk {}, {} saved", chunk->coords.x, chunk->coords.z));

    //if (chunk->mesh.vertexArrayObject == 150) {
    //    Logger::error("Because of chunkUnload c %d, %d :", chunk->coords.x, chunk->coords.z);
    //}
}


bool blockInChunk(const Chunk* chunk, glm::ivec3 local) {
    Logger::assertion(chunk);

    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}

void chunkMarkDirty(Chunk* chunk, glm::ivec3 local) {
    Logger::assertion(chunk);
    Logger::assertion(blockInChunk(chunk, local));

    chunk->dirty = true;

#ifdef USE_IGNORED
    chunk->ignored[local.y] |= 1 << local.x;
#endif
}

void chunkSetBlockRaw(Chunk* chunk, glm::ivec3 local, Block block) {
    Logger::assertion(chunk);

    if (!blockInChunk(chunk, local)) {
        Logger::warning("Block placed outside of chunk");
        return;
    }

    chunk->blocks[local.x][local.y][local.z] = block;
    chunkMarkDirty(chunk, local);
}

void chunkSetBlock(Chunk* chunk, glm::ivec3 local, Block block) {
    Logger::assertion(chunk);

    chunkSetBlockRaw(chunk, local, block);

    glm::ivec3 worldPoint = localToWorld(chunk->coords, local);

    for (int i = 0; i < DIRECTION_COUNT; ++i) {
        worldMarkDirty(chunk->world, worldPoint + directionToPoint(static_cast<Direction>(i)));
    }
}

Block chunkGetBlock(const Chunk* chunk, glm::ivec3 local) {
    Logger::assertion(chunk);

    if (!blockInChunk(chunk, local)) {
        return BLOCK_AIR;
    }
    return chunk->blocks[local.x][local.y][local.z];
}

static void drawChunkMesh(const Chunk* chunk, ShaderProgram& shader, const GPUMesh& mesh) {
    Logger::assertion(chunk);

    glm::mat4 transform = glm::mat4{1.f};
    transform = glm::translate(transform, glm::vec3{chunk->coords * (glm::ivec3)CHUNK_SIZE});
    //transform = glm::scale(transform, CHUNK_SIZE);
    //transform = glm::translate(transform, pointToVector3(chunk->coords));

    if (!chunk->loaded) {
        Logger::error(std::format("Drawing unloaded chunk with vao: {}", chunk->mesh->vertexArrayObject.object));
    }

    shader.setUniformMat4("model", transform);
    mesh.draw();

    if (chunk->world->showChunkBorders) {
        wireframeEnable();
        //DrawCubeWiresV(Vector3Multiply(Vector3AddValue(pointToVector3(chunk->coords), 0.5f), (Vector3)CHUNK_SIZE), (Vector3)CHUNK_SIZE, WHITE);
        wireframeDisable();
    }
}

void drawChunk(const Chunk* chunk, ShaderProgram& shader) {
    Logger::assertion(chunk);

    drawChunkMesh(chunk, shader, chunk->mesh.value());
}

void drawChunkTranslucent(const Chunk* chunk, ShaderProgram& shader) {
    Logger::assertion(chunk);

    drawChunkMesh(chunk, shader, chunk->translucentMesh.value());
}

bool verifyChunk(const Chunk* chunk) {
    Logger::assertion(chunk);

    Logger::debug(std::format("Verifying chunk {}, {}", chunk->coords.x, chunk->coords.z));

    ITERATE_CHUNK(x, y, z) {
        Block block = chunk->blocks[x][y][z];
        if (block >= BLOCK_COUNT) {
            Logger::error(std::format("Chunk verification failed. Chunk: {}, {}. Block: {}, {}, {}",
                    chunk->coords.x, chunk->coords.z, x, y, z));
            return false;
        }
    }
    return true;
}

void chunkComputeLightValues(Chunk* chunk) {
    (void)chunk;

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {

            }
        }
    }
}

