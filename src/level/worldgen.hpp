#ifndef WORLDGEN_HPP
#define WORLDGEN_HPP

#include "chunk.hpp"
#include <glm/geometric.hpp>

enum class Biome {
    desert,
    ocean,
    taiga,
    forest,
    plains,
    meadow,
    tundra,
    jungle,
};


void generateTerrain(Chunk* chunk);
void placeFeatures(Chunk* chunk);

#endif
