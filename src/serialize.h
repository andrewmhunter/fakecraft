#ifndef SERIALIZE_H
#define SERIALIZE_H

// DIGICAM

#include <stdbool.h>
#include <stdio.h>

struct Chunk;
struct World;
struct Entity;

void saveChunk(const struct Chunk* chunk);
bool loadChunk(struct Chunk* chunk);

void saveWorld(const struct World* world);
bool loadWorld(struct World* world);

void saveEntity(const struct Entity* entity, FILE* file);
void loadEntity(struct Entity* entity, FILE* file);

void makeSaveDirectories(void);

#endif

