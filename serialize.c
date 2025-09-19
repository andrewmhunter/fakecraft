#include <raylib.h>
#include <stdio.h>
#include "serialize.h"
#include "world.h"
#include "chunk.h"
#include "entity.h"
#include "logger.h"

void saveVector3(Vector3 vector, FILE* file) {
    fwrite(&vector.x, sizeof(float), 1, file);
    fwrite(&vector.y, sizeof(float), 1, file);
    fwrite(&vector.z, sizeof(float), 1, file);
}

void loadVector3(Vector3* vector, FILE* file) {
    fread(&vector->x, sizeof(float), 1, file);
    fread(&vector->y, sizeof(float), 1, file);
    fread(&vector->z, sizeof(float), 1, file);

}

static void getChunkFileName(const Chunk* chunk, char* buffer, size_t maxSize) {
    int count = snprintf(buffer, maxSize, "save/level/c%d.%d.bin", chunk->coords.x, chunk->coords.z);
    if (count >= (int)maxSize) {
        ERROR("Buffer not large enough for chunk filename");
    }
}

void makeSaveDirectories(void) {
    MakeDirectory("save");
    MakeDirectory(TextFormat("save/level"));
}

void saveChunk(const Chunk* chunk) {
    ASSERT(chunk);

    char fileName[FILENAME_MAX];
    getChunkFileName(chunk, fileName, sizeof(fileName));
    FILE* file = fopen(fileName, "wb");
    ASSERT(file);

    size_t blocksSize = sizeof(chunk->blocks);

    size_t count = fwrite(chunk->blocks, 1, blocksSize, file);
    ASSERT(count == blocksSize);

    fclose(file);
}

bool loadChunk(Chunk* chunk) {
    ASSERT(chunk);

    char fileName[FILENAME_MAX];
    getChunkFileName(chunk, fileName, sizeof(fileName));
    FILE* file = fopen(fileName, "rb");
    if (file == NULL) {
        return false;
    }

    size_t blocksSize = sizeof(chunk->blocks);

    TRACE("Loading blocks array. Chunk: %d, %d", chunk->coords.x, chunk->coords.z);
    size_t count = fread(chunk->blocks, 1, blocksSize, file);
    if (count != blocksSize) {
        ERROR("Chunk %d, %d is courupted", chunk->coords.x, chunk->coords.z);
        return false;
    }

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

    FILE* file = fopen("save/world.bin", "wb");
    ASSERT(file);

    fwrite(&world->seed, 1, sizeof(world->seed), file);
    saveEntity(world->player, file);

    fclose(file);
}

bool loadWorld(World* world) {
    ASSERT(world);

    FILE* file = fopen("save/world.bin", "rb");
    if (file == NULL) {
        return false;
    }

    DEBUG("Loading world from file");

    fread(&world->seed, 1, sizeof(world->seed), file);
    loadEntity(world->player, file);

    fclose(file);
    return true;
}


void saveEntity(const struct Entity* entity, FILE* file) {
    saveVector3(entity->position, file);
    saveVector3(entity->velocity, file);
    fwrite(&entity->yaw, sizeof(float), 1, file);
    fwrite(&entity->pitch, sizeof(float), 1, file);
}

void loadEntity(Entity* entity, FILE* file) {
    loadVector3(&entity->position, file);
    loadVector3(&entity->velocity, file);
    fread(&entity->yaw, sizeof(float), 1, file);
    fread(&entity->pitch, sizeof(float), 1, file);
}

