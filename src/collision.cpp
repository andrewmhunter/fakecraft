#include "collision.hpp"
#include "logger.hpp"
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <sys/cdefs.h>
#include <cmath>

constexpr float collisionEpsilon = 0.002;

WalkCollision::WalkCollision(glm::ivec3 startBlock, glm::vec3 startPosition)
    : blockAt{startBlock},
    blockBefore{startBlock},
    collisionAt{startPosition},
    collisionBefore{startPosition}
{}

// Digital differential analysis, extended to work in 3d
// https://www.youtube.com/watch?v=NbSee-XM7WA
// https://lodev.org/cgtutor/raycasting.html
WalkCollision ddaCastRay(const World* world, glm::vec3 start, glm::vec3 direction, float maxLength) {
    Logger::assertion(world);

    direction = glm::normalize(direction);

    glm::vec3 deltaDistance = {
        std::sqrt(1 + squaref(direction.y / direction.x) + squaref(direction.z / direction.x)),
        std::sqrt(1 + squaref(direction.x / direction.y) + squaref(direction.z / direction.y)),
        std::sqrt(1 + squaref(direction.x / direction.z) + squaref(direction.y / direction.z))
    };

    const glm::ivec3 worldPoint = vector3ToPoint(start);
    WalkCollision collision{worldPoint, start};


    glm::vec3 sideDistance{0.f};

    glm::ivec3 step{1};

    if (direction.x < 0) {
        step.x = -1;
        sideDistance.x = (start.x - std::floor(worldPoint.x)) * deltaDistance.x;
    } else {
        sideDistance.x = (std::floorf(worldPoint.x) - start.x + 1.f) * deltaDistance.x;
    } 

    if (direction.y < 0) {
        step.y = -1;
        sideDistance.y = (start.y - std::floor(worldPoint.y)) * deltaDistance.y;
    } else {
        sideDistance.y = (floorf(worldPoint.y) - start.y + 1.f) * deltaDistance.y;
    } 

    if (direction.z < 0) {
        step.z = -1;
        sideDistance.z = (start.z - std::floor(worldPoint.z)) * deltaDistance.z;
    } else {
        sideDistance.z = (std::floorf(worldPoint.z) - start.z + 1.f) * deltaDistance.z;
    }

    collision.collided = false;

    while (collision.length < maxLength) {
        if (world->getBlock(collision.blockAt) != BLOCK_AIR) {
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

    collision.lengthBefore = std::max(collision.length - 0.0001f, 0.f);
    collision.distance = direction * collision.length;
    collision.distanceBefore = direction * collision.lengthBefore;
    collision.collisionAt = start + collision.distance;
    collision.collisionBefore = start + collision.distanceBefore;
    return collision;
}

BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max)
    : min{min}, max{max}
{}

BoundingBox genBoundingBox(glm::vec3 position, glm::vec3 bounds) {
    glm::vec3 sides{bounds.x / 2, 0.f, bounds.z / 2.f};
    glm::vec3 min = position - sides;
    sides.y = bounds.y;
    glm::vec3 max = position + sides;
    return BoundingBox{min, max};
}


// https://medium.com/@andrebluntindie/3d-aabb-collision-detection-and-resolution-for-voxel-games-5fcbfdb8cdb4

static constexpr bool overlapsAxis(BoundingBox boundingBox, glm::vec3 block, int axis, float offset = 0.f) {
    return boundingBox.max[axis] > (block[axis] + offset) && boundingBox.min[axis] < (block[axis] + 1 - offset);
}

bool isPassable(Block block) {
    return blocks[block]->passability == PASSABLE;
}

static float aabbResolveAxisBlock(const World* world, BoundingBox boundingBox, glm::vec3 velocity, glm::ivec3 blockPosition, int axis) {
    const int axis1 = (axis + 1) % 3;
    const int axis2 = (axis + 2) % 3;

    // If the block can be walked through we don't
    // care if the bounding box is colliding
    if (isPassable(world->getBlock(blockPosition))) {
        return velocity[axis];
    }

    glm::vec3 worldVector = glm::vec3{blockPosition};

    // The final target bounding box for this moving entity
    BoundingBox bb{
        boundingBox.min + velocity,
        boundingBox.max + velocity
    };

    bool intersects0 = overlapsAxis(boundingBox, worldVector, axis);
    bool intersects1 = overlapsAxis(boundingBox, worldVector, axis1);
    bool intersects2 = overlapsAxis(boundingBox, worldVector, axis2);

    // If the entity is already inside a block allow it move around to get out
    if (intersects0 && intersects1 && intersects2) {
        return velocity[axis];
    }

    bool willIntersect0 = overlapsAxis(bb, worldVector, axis, -collisionEpsilon);

    if (intersects1 && intersects2 && willIntersect0) {
        return 0.f;
        if (velocity[axis] < 0.f) {
            velocity[axis] = std::min(worldVector[axis] + 1.f - boundingBox.min[axis] + collisionEpsilon, 0.f);
        } else {
            velocity[axis] = std::max(worldVector[axis] - boundingBox.max[axis] - collisionEpsilon, 0.f);
        }
    }

    return velocity[axis];
}

static inline constexpr float absMinf(float a, float b) {
    if (std::fabs(a) <= std::fabs(b)) {
        return a;
    }
    return b;
}

static void aabbResolveAxis(const World* world, BoundingBox& boundingBox, glm::vec3& velocity, int axis) {
    // One is added or subtracted to the positions to round them up using truncate
    glm::vec3 blockOffset{1 + collisionEpsilon};
    glm::ivec3 mins = glm::ivec3{glm::trunc(glm::min(boundingBox.min + velocity, boundingBox.min) - blockOffset)};
    glm::ivec3 maxes = glm::ivec3{glm::trunc(glm::max(boundingBox.max + velocity, boundingBox.max) + blockOffset)};

    for (int x = mins.x; x <= maxes.x; ++x) {
        for (int y = mins.y; y <= maxes.y; ++y) {
            for (int z = mins.z; z <= maxes.z; ++z) {
                // glm::ivec3 blockPosition = vector3ToPoint(position) + glm::ivec3{x, y, z};
                glm::ivec3 blockPosition{x, y, z};
                float resolved = aabbResolveAxisBlock(world, boundingBox, velocity, blockPosition, axis);
                velocity[axis] = absMinf(velocity[axis], resolved);
            }
        }
    }

    boundingBox.min[axis] += velocity[axis];
    boundingBox.max[axis] += velocity[axis];
}

glm::vec3 aabbResolveCollisions(const World* world, glm::vec3 position, glm::vec3 bounds, glm::vec3 velocity) {
    BoundingBox boundingBox = genBoundingBox(position, bounds);

    for (int axis = 0; axis < 3; ++axis) {
        aabbResolveAxis(world, boundingBox, velocity, axis);
    }

    return velocity;
}
