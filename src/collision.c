#include "collision.h"
#include "logger.h"
#include <raymath.h>

// Digital differential analysis, extended to work in 3d
// https://www.youtube.com/watch?v=NbSee-XM7WA
// https://lodev.org/cgtutor/raycasting.html
WalkCollision ddaCastRay(const World* world, Vector3 start, Vector3 direction, float maxLength) {
    ASSERT(world);

    direction = Vector3Normalize(direction);

    Vector3 deltaDistance = {
        sqrtf(1 + squaref(direction.y / direction.x) + squaref(direction.z / direction.x)),
        sqrtf(1 + squaref(direction.x / direction.y) + squaref(direction.z / direction.y)),
        sqrtf(1 + squaref(direction.x / direction.z) + squaref(direction.y / direction.z))
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

BoundingBox genBoundingBox(Vector3 position, Vector3 bounds) {
    Vector3 sides = {bounds.x / 2, 0.f, bounds.z / 2.f};
    Vector3 min = Vector3Subtract(position, sides);
    sides.y = bounds.y;
    Vector3 max = Vector3Add(position, sides);
    return (BoundingBox) {.min = min, .max = max};
}


// https://medium.com/@andrebluntindie/3d-aabb-collision-detection-and-resolution-for-voxel-games-5fcbfdb8cdb4

static bool overlapsX(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.x > block.x && boundingBox.min.x < block.x + 1;
}

static bool overlapsY(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.y > block.y && boundingBox.min.y < block.y + 1;
}

static bool overlapsZ(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.z > block.z && boundingBox.min.z < block.z + 1;
}

#if 0
static inline bool overlapsAxis(BoundingBox boundingBox, Vector3 block, Axis axis) {
    return vector3GetAxis(boundingBox.max, axis) > vector3GetAxis(block, axis)
        && vector3GetAxis(boundingBox.min, axis) < vector3GetAxis(block, axis) + 1;
}

static inline float aabbResolveAxis(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition, Axis axis) {
    float velocityA = vector3GetAxis(velocity, axis);

    if (worldGetBlock(world, blockPosition) == BLOCK_AIR) {
        return velocityA;
    }

    Vector3 worldVector = pointToVector3(blockPosition);
    float worldVectorA = vector3GetAxis(worldVector, axis);

    BoundingBox movedBounds = {
        .min = Vector3Add(boundingBox.min, velocity),
        .max = Vector3Add(boundingBox.max, velocity)
    };

    bool intersectsA = overlapsAxis(boundingBox, worldVector, axis);
    bool intersectsB = overlapsAxis(boundingBox, worldVector, axisOther0(axis));
    bool intersectsC = overlapsAxis(boundingBox, worldVector, axisOther1(axis));

    if (intersectsA && intersectsB && intersectsC) {
        return velocityA;
    }

    bool willIntersectA = overlapsAxis(movedBounds, worldVector, axis);

    if (intersectsB && intersectsC && willIntersectA) {
        if (velocityA < 0.f && vector3GetAxis(movedBounds.min, axis) <= worldVectorA + 1.f) {
            velocityA = MIN(worldVectorA + 1.f - vector3GetAxis(boundingBox.min, axis) + EPSILON, 0.f);
        } else if (velocityA > 0.f && vector3GetAxis(movedBounds.max, axis) >= worldVectorA) {
            velocityA = MAX(worldVectorA - vector3GetAxis(boundingBox.max, axis) - EPSILON, 0.f);
        }
    }

    return velocityA;
}
#endif

static float aabbResolveX(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
    if (worldGetBlock(world, blockPosition) == BLOCK_AIR) {
        return velocity.x;
    }

    Vector3 worldVector = pointToVector3(blockPosition);

    BoundingBox bb = {
        .min = Vector3Add(boundingBox.min, velocity),
        .max = Vector3Add(boundingBox.max, velocity)
    };

    bool intersectsX = overlapsX(boundingBox, worldVector);
    bool intersectsY = overlapsY(boundingBox, worldVector);
    bool intersectsZ = overlapsZ(boundingBox, worldVector);

    if (intersectsX && intersectsY && intersectsZ) {
        return velocity.x;
    }

    bool willIntersectX = overlapsX(bb, worldVector);

    if (intersectsY && intersectsZ && willIntersectX) {
        if (velocity.x < 0.f && bb.min.x <= worldVector.x + 1.f) {
            velocity.x = MIN(worldVector.x + 1.f - boundingBox.min.x + EPSILON, 0.f);
        } else if (velocity.x > 0.f && bb.max.x >= worldVector.x) {
            velocity.x = MAX(worldVector.x - boundingBox.max.x - EPSILON, 0.f);
        }
    }

    return velocity.x;
}

static float aabbResolveY(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
    if (worldGetBlock(world, blockPosition) == BLOCK_AIR) {
        return velocity.y;
    }

    Vector3 worldVector = pointToVector3(blockPosition);

    BoundingBox bb = {
        .min = Vector3Add(boundingBox.min, velocity),
        .max = Vector3Add(boundingBox.max, velocity)
    };

    bool intersectsX = overlapsX(boundingBox, worldVector);
    bool intersectsY = overlapsY(boundingBox, worldVector);
    bool intersectsZ = overlapsZ(boundingBox, worldVector);

    if (intersectsX && intersectsY && intersectsZ) {
        return velocity.y;
    }

    bool willIntersectY = overlapsY(bb, worldVector);

    if (intersectsX && intersectsZ && willIntersectY) {
        if (velocity.y < 0.f && bb.min.y <= worldVector.y + 1.f) {
            velocity.y = MIN(worldVector.y + 1.f - boundingBox.min.y + EPSILON, 0.f);
        } else if (velocity.y > 0.f && bb.max.y >= worldVector.y) {
            velocity.y = MAX(worldVector.y - boundingBox.max.y - EPSILON, 0.f);
        }
    }

    return velocity.y;
}

static float aabbResolveZ(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
    if (worldGetBlock(world, blockPosition) == BLOCK_AIR) {
        return velocity.z;
    }

    Vector3 worldVector = pointToVector3(blockPosition);

    BoundingBox bb = {
        .min = Vector3Add(boundingBox.min, velocity),
        .max = Vector3Add(boundingBox.max, velocity)
    };

    bool intersectsX = overlapsX(boundingBox, worldVector);
    bool intersectsY = overlapsY(boundingBox, worldVector);
    bool intersectsZ = overlapsZ(boundingBox, worldVector);

    if (intersectsX && intersectsY && intersectsZ) {
        return velocity.z;
    }

    bool willIntersectZ = overlapsZ(bb, worldVector);

    if (intersectsX && intersectsY && willIntersectZ) {
        if (velocity.z < 0.f && bb.min.z <= worldVector.z + 1.f) {
            velocity.z = MIN(worldVector.z + 1.f - boundingBox.min.z + EPSILON, 0.f);
        } else if (velocity.z > 0.f && bb.max.z > worldVector.z) {
            velocity.z = MAX(worldVector.z - boundingBox.max.z - EPSILON, 0.f);
        }
    }

    return velocity.z;
}

static inline float absMinf(float a, float b) {
    if (fabs(a) <= fabs(b)) {
        return a;
    }
    return b;
}

Vector3 aabbResolveCollisions(const World* world, Vector3 position, Vector3 bounds, Vector3 velocity) {
    BoundingBox boundingBox = genBoundingBox(position, bounds);
    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 4; ++j) {
            for (int k = -2; k <= 2; ++k) {
                Point blockPosition = pointAdd(vector3ToPoint(position), point(i, j, k));
                float resolved = aabbResolveX(world, boundingBox, velocity, blockPosition);
                velocity.x = absMinf(velocity.x, resolved);
            }
        }
    }

    boundingBox.min.x += velocity.x;
    boundingBox.max.x += velocity.x;

    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 4; ++j) {
            for (int k = -2; k <= 2; ++k) {
                Point blockPosition = pointAdd(vector3ToPoint(position), point(i, j, k));
                float resolved = aabbResolveY(world, boundingBox, velocity, blockPosition);
                velocity.y = absMinf(velocity.y, resolved);
            }
        }
    }

    boundingBox.min.y += velocity.y;
    boundingBox.max.y += velocity.y;

    for (int i = -2; i <= 2; ++i) {
        for (int j = -2; j <= 4; ++j) {
            for (int k = -2; k <= 2; ++k) {
                Point blockPosition = pointAdd(vector3ToPoint(position), point(i, j, k));
                float resolved = aabbResolveZ(world, boundingBox, velocity, blockPosition);
                velocity.z = absMinf(velocity.z, resolved);
            }
        }
    }

    return velocity;
}

