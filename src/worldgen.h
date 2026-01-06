#ifndef WORLDGEN_H
#define WORLDGEN_H

#include "chunk.h"

typedef enum {
    FEATURE_NUMBER = 1331251,
    FEATURE_INDEX = 3867931,
    FEATURE_FLAG = 1114471,
    FEATURE_X = 6450893,
    FEATURE_Y = 6167971,
    FEATURE_Z = 1334233,
    FEATURE_TREE = 5876579,
    FEATURE_DUNGEON = 2585357,
    FEATURE_FLOWER_PATCH = 8944487,
    FEATURE_TERRAIN = 6066163,
    
    /*5544101
    5135621
    9894523
    1580357
    3566513
    7855357
    2331647
    8373293
    1375133
    1557623*/
} FeatureId;

Hash featureSeed(Hash seed, FeatureId feature);
int seededNumber(Hash seed, FeatureId feature, int min, int max);

void generateTerrain(Chunk* chunk);
void placeFeatures(Chunk* chunk);

#endif
