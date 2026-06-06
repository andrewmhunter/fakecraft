#ifndef COLLISION_HPP
#define COLLISION_HPP

#include "world.hpp"

struct FCBoundingBox {
    glm::vec3 min;
    glm::vec3 max;
};

typedef struct {
    glm::ivec3 blockAt;
    glm::ivec3 blockBefore;
    glm::vec3 collisionAt;
    glm::vec3 collisionBefore;
    glm::vec3 distance;
    glm::vec3 distanceBefore;
    float length;
    float lengthBefore;
    bool collided;
} WalkCollision;

WalkCollision ddaCastRay(const World* world, glm::vec3 start, glm::vec3 direction, float maxLength);
glm::vec3 aabbResolveCollisions(const World* world, glm::vec3 position, glm::vec3 bounds, glm::vec3 velocity);
FCBoundingBox genBoundingBox(glm::vec3 position, glm::vec3 bounds);

#endif
