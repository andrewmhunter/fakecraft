#include "collision.h"
#include "logger.h"

// Digital differential analysis, extended to work in 3d
// https://www.youtube.com/watch?v=NbSee-XM7WA
// https://lodev.org/cgtutor/raycasting.html
WalkCollision ddaCastRay(const World* world, Vector3 start, Vector3 direction, float maxLength) {
    ASSERT(world);
    //ASSERT(Vector3LengthSqr(direction) > 0);

    direction = Vector3Normalize(direction);

    Vector3 deltaDistance = {
        sqrt(1 + squaref(direction.y / direction.x) + squaref(direction.z / direction.x)),
        sqrt(1 + squaref(direction.x / direction.y) + squaref(direction.z / direction.y)),
        sqrt(1 + squaref(direction.x / direction.z) + squaref(direction.y / direction.z))
    };

    const Point worldPoint = vector3ToPoint(start);
    WalkCollision collision = {
        .blockAt = worldPoint,
        .blockBefore = worldPoint,
        .collided = false,
        .collisionAt = start,
        .length = 0.f,
        .lengthBefore = 0.f
    };


    Vector3 sideDistance = Vector3Zero();

    Point step = {1.f, 1.f, 1.f};

    if (direction.x < 0) {
        step.x = -1;
        sideDistance.x = (start.x - floor(worldPoint.x)) * deltaDistance.x;
    } else {
        sideDistance.x = (floorf(worldPoint.x) - start.x + 1.f) * deltaDistance.x;
    } 

    if (direction.y < 0) {
        step.y = -1;
        sideDistance.y = (start.y - floor(worldPoint.y)) * deltaDistance.y;
    } else {
        sideDistance.y = (floorf(worldPoint.y) - start.y + 1.f) * deltaDistance.y;
    } 

    if (direction.z < 0) {
        step.z = -1;
        sideDistance.z = (start.z - floor(worldPoint.z)) * deltaDistance.z;
    } else {
        sideDistance.z = (floorf(worldPoint.z) - start.z + 1.f) * deltaDistance.z;
    }

    collision.collided = false;

    while (collision.length < maxLength) {
        if (worldGetBlock(world, collision.blockAt) != BLOCK_AIR) {
            collision.collided = true;
            break;
        }

        collision.blockBefore = collision.blockAt;

        if (sideDistance.x < sideDistance.y && sideDistance.x < sideDistance.z) {
            collision.blockAt.x += step.x;
            collision.length = sideDistance.x;
            sideDistance.x += deltaDistance.x;
        } else if (sideDistance.y < sideDistance.z) {
            collision.blockAt.y += step.y;
            collision.length = sideDistance.y;
            sideDistance.y += deltaDistance.y;
        } else {
            collision.blockAt.z += step.z;
            collision.length = sideDistance.z;
            sideDistance.z += deltaDistance.z;
        }
        
    }

    collision.lengthBefore = MAX(collision.length - 0.0001f, 0.f);
    collision.distance = Vector3Scale(direction, collision.length);
    collision.distanceBefore = Vector3Scale(direction, collision.lengthBefore);
    collision.collisionAt = Vector3Add(start, collision.distance);
    collision.collisionBefore = Vector3Add(start, collision.distanceBefore);
    return collision;
}

WalkCollision worldWalkRay(const World* world, Vector3 start, Vector3 direction, float maxLength) {
    ASSERT(world);

    direction = Vector3Normalize(direction);
    float epsilon = 0.05;
    Vector3 step = Vector3Scale(direction, epsilon);

    WalkCollision coll;
    coll.collided = false;
    Vector3 before = start;
    coll.collisionAt = start;

    Vector3 walk = {0, 0, 0};
    while (Vector3Length(walk) <= maxLength) {
        before = coll.collisionAt;
        walk = Vector3Add(walk, step);
        coll.collisionAt = Vector3Add(start, walk);
        if (worldGetBlock(world, vector3ToPoint(coll.collisionAt)) != BLOCK_AIR) {
            coll.collided = true;
            break;
        }
    }

    coll.blockAt = vector3ToPoint(coll.collisionAt);
    coll.blockBefore = vector3ToPoint(before);
    return coll;
}

