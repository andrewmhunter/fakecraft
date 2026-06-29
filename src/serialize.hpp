#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

// DIGICAM

#include <stdbool.h>
#include <stdio.h>

class Chunk;
class World;
class Entity;

void saveChunk(const Chunk* chunk);
bool loadChunk(Chunk* chunk);

void saveWorld(const World* world);
bool loadWorld(World* world);

void saveEntity(const Entity* entity, FILE* file);
void loadEntity(Entity* entity, FILE* file);

void makeSaveDirectories(void);

#endif

