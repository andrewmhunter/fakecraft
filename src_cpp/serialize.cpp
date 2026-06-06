#include <stdio.h>
#include <stdint.h>
#include <filesystem>
#include "serialize.hpp"
#include "world.hpp"
#include "chunk.hpp"
#include "entity.hpp"
#include "logger.hpp"
#include "fileio.hpp"

void saveFloat(float number, FILE* file) {
    ASSERT(fwrite(&number, sizeof(number), 1, file) == 1);
}

float loadFloat(FILE* file) {
    float number = 0.f;
    ASSERT(fread(&number, sizeof(number), 1, file) == 1);
    return number;
}

void saveI32(int32_t number, FILE* file) {
    ASSERT(fwrite(&number, sizeof(number), 1, file) == 1);
}

int32_t loadI32(FILE* file) {
    int32_t number = 0;
    ASSERT(fread(&number, sizeof(number), 1, file) == 1);
    return number;
}

void saveVector3(glm::vec3 vector, FILE* file) {
    saveFloat(vector.x, file);
    saveFloat(vector.y, file);
    saveFloat(vector.z, file);
}

glm::vec3 loadVector3(FILE* file) {
    glm::vec3 vector = {0.f, 0.f, 0.f};
    vector.x = loadFloat(file);
    vector.y = loadFloat(file);
    vector.z = loadFloat(file);
    return vector;
}


static void getChunkFileName(const Chunk* chunk, char* buffer, size_t maxSize) {
    int count = snprintf(buffer, maxSize, "save/level/c%d.%d.bin", chunk->coords.x, chunk->coords.z);
    if (count >= (int)maxSize) {
        ERROR("Buffer not large enough for chunk filename");
    }
}

void makeSaveDirectories(void) {
    std::filesystem::create_directories("save/level");
}

void saveChunk(const Chunk* chunk) {

#ifdef NO_SAVE_CHUNKS
    return;
#endif

    ASSERT(chunk);

    TRACE("Saving chunk: %d, %d", chunk->coords.x, chunk->coords.z);

    char fileName[FILENAME_MAX];
    getChunkFileName(chunk, fileName, sizeof(fileName));
    FILE* file = openFileRequired(fileName, "wb");

    size_t blocksSize = sizeof(chunk->blocks);
    size_t savedCount = 0;

    int currentNumber = 0;
    Block currentBlock = chunk->blocks[0][0][0];

    ITERATE_CHUNK_YXZ(x, y, z) {
        Block block = chunk->blocks[x][y][z];
        if (block != currentBlock || currentNumber == UINT8_MAX + 1) {
            fputc(currentNumber - 1, file);
            fputc(currentBlock, file);
            currentNumber = 0;
            currentBlock = block;
        }
        currentNumber++;
        savedCount++;
    }

    if (currentNumber != 0) {
        fputc(currentNumber - 1, file);
        fputc(currentBlock, file);
    }

    ASSERT(savedCount == blocksSize);

    fclose(file);
}

bool loadChunk(Chunk* chunk) {
    ASSERT(chunk);

    char fileName[FILENAME_MAX];
    getChunkFileName(chunk, fileName, sizeof(fileName));

    FILE* file = NULL;
    if (!openFile(&file, fileName, "rb", LOG_TRACE)) {
        return false;
    }

    //size_t blocksSize = sizeof(chunk->blocks);

    TRACE("Loading chunk: %d, %d", chunk->coords.x, chunk->coords.z);

    int currentNumber = 0;
    Block currentBlock = BLOCK_AIR;

    ITERATE_CHUNK_YXZ(x, y, z) {
        if (currentNumber <= 0) {
            currentNumber = fgetc(file) + 1;
            int blockId = fgetc(file);

            if (currentNumber == EOF || blockId == EOF) {
                ERROR("Chunk %d, %d is courupted", chunk->coords.x, chunk->coords.z);
                return false;
            }

            currentBlock = static_cast<Block>(blockId);
        }

        chunk->blocks[x][y][z] = currentBlock;
        currentNumber--;
    }

    /*if (count != blocksSize) {
        ERROR("Chunk %d, %d is courupted", chunk->coords.x, chunk->coords.z);
        return false;
    }*/

    fclose(file);

    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            chunk->surfaceHeight[x][z] = 65;
        }
    }

    chunk->dirty = true;
    return verifyChunk(chunk);
}

void saveWorld(const World* world) {
    ASSERT(world);

    DEBUG("Saving world to file");

    FILE* file = openFileRequired("save/world.bin", "wb");

    saveI32(world->seed, file);
    saveEntity(world->player, file);

    for (const Entity* entity : world->entities) {
        if (entity->type == ENTITY_PLAYER) {
            continue;
        }

        fputc(1, file);
        saveEntity(entity, file);
    }

    fputc(0, file);

    fclose(file);
}

bool loadWorld(World* world) {
    ASSERT(world);

    FILE* file = NULL;
    if (!openFile(&file, "save/world.bin", "rb", LOG_WARNING)) {
        return false;
    }

    DEBUG("Loading world from file");

    world->seed = loadI32(file);
    loadEntity(world->player, file);

    int ch = 0;
    while ((ch = fgetc(file)) == 1) {
        Entity* entity = spawnEntity(world, ENTITY_MOB, glm::vec3{0.f}, 0.6, 1.8);
        loadEntity(entity, file);
    }

    fclose(file);
    return true;
}


void saveEntity(const struct Entity* entity, FILE* file) {
    saveVector3(entity->position, file);
    saveVector3(entity->velocity, file);
    saveFloat(entity->yaw, file);
    saveFloat(entity->pitch, file);
}

void loadEntity(Entity* entity, FILE* file) {
    entity->position = loadVector3(file);
    entity->velocity = loadVector3(file);
    entity->yaw = loadFloat(file);
    entity->pitch = loadFloat(file);
}

