#include "entity.h"
#include "collision.h"
#include <raymath.h>

/*typedef struct {
    Point collides;
    Vector3 maxMotion;
    Vector3 newVelocity;
} BlockCollision;

BlockCollision aabbCollideBlocks(const World* world, Vector3 position, Vector3 boundingBox, Vector3 velocity) {
    Vector3 axisVelocity = 
    int blockSpan = ceil(boundingBox.x) + 1;
    float increment = boundingBox.x / (blockSpan - 1);

    for (int i = 0; i < blockSpan; ++i) {
    }

}*/

static inline Vector3 vector3Abs(Vector3 vector) {
    return (Vector3){fabs(vector.x), fabs(vector.y), fabs(vector.z)};
}

static Vector3 vector3SmallestAbs(Vector3 vector0, Vector3 vector1) {
    if (fabs(vector1.x) < fabs(vector0.x)) {
        vector0.x = vector1.x;
    }
    if (fabs(vector1.y) < fabs(vector0.y)) {
        vector0.y = vector1.y;
    }
    if (fabs(vector1.z) < fabs(vector0.z)) {
        vector0.z = vector1.z;
    }
    return vector0;
}

Vector3 aabbIntersectsSolid(World* world, Vector3 position, Vector3 boundingBox, Vector3 velocity) {
    assert(world);

    float velocityScale = Vector3Length(velocity);

    int blockSpanX = ceil(boundingBox.x);
    float incrementX = boundingBox.x / blockSpanX;

    for (int i = 0; i <= blockSpanX; ++i) {
        float x = position.x - boundingBox.x / 2 + incrementX * i;

        int blockSpanY = ceil(boundingBox.y);
        float incrementY = boundingBox.y / blockSpanY;

        for (int j = 0; j <= blockSpanY; ++j) {
            float y = position.y + incrementY * j;

            int blockSpanZ = ceil(boundingBox.z);
            float incrementZ = boundingBox.z / blockSpanZ;

            for (int k = 0; k <= blockSpanZ; ++k) {
                float z = position.z - boundingBox.z / 2 + incrementZ * k;

                Vector3 worldPosition = {x, y, z};

                WalkCollision collision = ddaCastRay(world, worldPosition, velocity, velocityScale);

                velocity = vector3SmallestAbs(velocity, collision.distanceBefore);
                if (collision.collided) {
                    velocity = Vector3Zero();
                }
            }
        }
    }

    return velocity;
}

BoundingBox genBoundingBox(Vector3 position, Vector3 bounds) {
    Vector3 sides = {bounds.x / 2, 0.f, bounds.z / 2.f};
    Vector3 min = Vector3Subtract(position, sides);
    sides.y = bounds.y;
    Vector3 max = Vector3Add(position, sides);
    return (BoundingBox) {.min = min, .max = max};
}


// https://medium.com/@andrebluntindie/3d-aabb-collision-detection-and-resolution-for-voxel-games-5fcbfdb8cdb4

bool overlapsX(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.x > block.x && boundingBox.min.x < block.x + 1;
}

bool overlapsY(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.y > block.y && boundingBox.min.y < block.y + 1;
}

bool overlapsZ(BoundingBox boundingBox, Vector3 block) {
    return boundingBox.max.z > block.z && boundingBox.min.z < block.z + 1;
}

float aabbResolveX(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
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

float aabbResolveY(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
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

bool collides = false;
Vector3 position = {0};

float aabbResolveZ(const World* world, BoundingBox boundingBox, Vector3 velocity, Point blockPosition) {
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

    bool isTarget = (blockPosition.z == 0 || blockPosition.z == -1);

    if (intersectsX && intersectsY && willIntersectZ) {
        if (velocity.z < 0.f && bb.min.z <= worldVector.z + 1.f) {
            collides = true;
            //printf("vz: %f -> ", velocity.z);

            velocity.z = MIN(worldVector.z + 1.f - boundingBox.min.z + EPSILON, 0.f);

            //printf("%f, z: %.010f\n", velocity.z, position.z + velocity.z);
            //printf("f: %.010f\n", worldVector.z + 1.f + 0.3f);
        } else if (velocity.z > 0.f && bb.max.z > worldVector.z) {
            velocity.z = MAX(worldVector.z - boundingBox.max.z - EPSILON, 0.f);
        }
    }

    return velocity.z;
}

inline float absMinf(float a, float b) {
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

void entityInit(Entity* entity, EntityType type, World* world, Vector3 position, float width, float height) {
    assert(entity);
    assert(world);

    entity->type = type;
    entity->world = world;
    entity->position = position;
    entity->velocity = Vector3Zero();
    entity->velocityOld = Vector3Zero();
    entity->boundingBox = (Vector3){width, height, width};
}

void entityUnload(Entity* entity) {
    assert(entity);
}

#define UPDATE_POSITION_FIRST

void entityUpdate(Entity* entity, float deltaTime) {
    assert(entity);

    // Minecraft entity physics: https://minecraft.wiki/w/Entity#Motion
    // https://gamedev.stackexchange.com/questions/169558/how-can-i-fix-my-velocity-damping-to-work-with-any-delta-frame-time

#ifdef UPDATE_POSITION_FIRST
    Vector3 avgVelocity = Vector3Scale(Vector3Add(entity->velocity, entity->velocityOld), deltaTime * 0.5f);
    //Vector3 avgVelocity = Vector3Scale(entity->velocity, );
    Vector3 baseVelocity = avgVelocity;

    collides = false;

    position = entity->position;
    avgVelocity = aabbResolveCollisions(entity->world, entity->position, entity->boundingBox, avgVelocity);
    //printf("%s\n", formatVector3(Vector3Scale(avgVelocity, GetFPS())));

    entity->position = Vector3Add(entity->position, avgVelocity);

    if (avgVelocity.x != baseVelocity.x) {
        entity->velocity.x = 0.f;
    }

    if (avgVelocity.y != baseVelocity.y) {
        entity->velocity.y = 0.f;
    }

    if (avgVelocity.z != baseVelocity.z) {
        entity->velocity.z = 0.f;
    }


    entity->velocityOld = entity->velocity;
#endif

    float horizontalDrag = 0.546f;
    //float horizontalDrag = 0.91f;
    float verticalDrag = 0.98f;


    entity->velocity.y -= (double)0.08 * deltaTime;

    //horizontalDrag = 0.8f;
    //verticalDrag = 0.8f;

    horizontalDrag = 1.f - powf(horizontalDrag, deltaTime);
    verticalDrag = 1.f - powf(verticalDrag, deltaTime);

    entity->velocity.x = Lerp(entity->velocity.x, 0.f, horizontalDrag);
    entity->velocity.y = Lerp(entity->velocity.y, 0.f, verticalDrag);//* deltaTime * 20;
    entity->velocity.z = Lerp(entity->velocity.z, 0.f, horizontalDrag);

#ifndef UPDATE_POSITION_FIRST
    Vector3 avgVelocity = Vector3Scale(entity->velocity, deltaTime * 0.5f);
    //Vector3 avgVelocity = Vector3Scale(entity->velocity, );
    Vector3 baseVelocity = avgVelocity;

    collides = false;

    position = entity->position;
    avgVelocity = aabbResolveCollisions(entity->world, entity->position, entity->boundingBox, avgVelocity);
    printf("%s\n", formatVector3(Vector3Scale(avgVelocity, GetFPS())));

    entity->position = Vector3Add(entity->position, avgVelocity);

    if (avgVelocity.x != baseVelocity.x) {
        entity->velocity.x = 0.f;
    }

    if (avgVelocity.y != baseVelocity.y) {
        entity->velocity.y = 0.f;
    }

    if (avgVelocity.z != baseVelocity.z) {
        entity->velocity.z = 0.f;
    }


    entity->velocityOld = entity->velocity;
#endif
}

void entityDraw(const Entity* entity) {
    assert(entity);

    DrawBoundingBox(genBoundingBox(entity->position, entity->boundingBox), WHITE);
}

