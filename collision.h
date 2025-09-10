#ifndef COLLISION_H
#define COLLISION_H

#include <raylib.h>
#include "util.h"
#include "world.h"

typedef struct {
    Point blockAt;
    Point blockBefore;
    Vector3 collisionAt;
    Vector3 collisionBefore;
    Vector3 distance;
    Vector3 distanceBefore;
    float length;
    float lengthBefore;
    bool collided;
} WalkCollision;

WalkCollision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength);
WalkCollision ddaCastRay(const World* world, Vector3 start, Vector3 direction, float maxLength);

#endif
