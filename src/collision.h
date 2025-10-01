#ifndef COLLISION_H
#define COLLISION_H

#include <raylib.h>
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

WalkCollision ddaCastRay(const World* world, Vector3 start, Vector3 direction, float maxLength);
Vector3 aabbResolveCollisions(const World* world, Vector3 position, Vector3 bounds, Vector3 velocity);
BoundingBox genBoundingBox(Vector3 position, Vector3 bounds);

#endif
