#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>

class World;

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    BoundingBox(glm::vec3 min, glm::vec3 max);

    constexpr bool intersectsAxis(const BoundingBox& other, int axis) {
        return min[axis] < other.max[axis] && max[axis] > other.min[axis];
    }

    constexpr bool intersects(const BoundingBox& other) const {
        return min.x < other.max.x && max.x > other.min.x
            && min.y < other.max.y && max.y > other.min.y
            && min.z < other.max.z && max.z > other.min.z;
    }
};

struct WalkCollision {
    glm::ivec3 blockAt {};
    glm::ivec3 blockBefore {};
    glm::vec3 collisionAt {};
    glm::vec3 collisionBefore {};
    glm::vec3 distance {};
    glm::vec3 distanceBefore {};
    float length = 0.f;
    float lengthBefore = 0.f;
    bool collided = false;

    WalkCollision(glm::ivec3 startBlock, glm::vec3 startPosition);
};

WalkCollision ddaCastRay(const World* world, glm::vec3 start, glm::vec3 direction, float maxLength);
glm::vec3 aabbResolveCollisions(const World* world, glm::vec3 position, glm::vec3 bounds, glm::vec3 velocity);
BoundingBox genBoundingBox(glm::vec3 position, glm::vec3 bounds);

#endif
