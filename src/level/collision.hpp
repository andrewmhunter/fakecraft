#ifndef COLLISION_HPP
#define COLLISION_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <limits>

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

    constexpr BoundingBox transformed(glm::mat4 transformation) const {
        glm::vec3 finalMin{std::numeric_limits<float>::infinity()};
        glm::vec3 finalMax{-std::numeric_limits<float>::infinity()};

        for (int x = 0; x <= 1; ++x) {
            for (int y = 0; y <= 1; ++y) {
                for (int z = 0; z <= 1; ++z) {
                    glm::vec3 minAxis{x, y, z};
                    glm::vec3 maxAxis = (minAxis - glm::vec3{1.f}) * -1.f;

                    glm::vec3 position = minAxis * min + maxAxis * max;

                    glm::vec3 transformed = transformation * glm::vec4{position, 1.f};
                    
                    finalMin = glm::min(finalMin, transformed);
                    finalMax = glm::max(finalMax, transformed);
                }
            }
        }
        return BoundingBox{finalMin, finalMax};
    }

    constexpr glm::vec3 getCenter() const {
        return (min + max) / 2.f;
    }

    constexpr glm::vec3 getExtents() const {
        return max - getCenter();
    }
};

struct Plane {
    glm::vec3 normal;
    float distance;

    Plane(glm::vec3 normal, float distance);
    Plane(glm::vec3 point, glm::vec3 normal);

    float getSignedDistance(glm::vec3 point) const;

    constexpr bool isForward(const BoundingBox& boundingBox) const {
        // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
        // https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling

        glm::vec3 extents = boundingBox.getExtents();

        float projection = extents.x * glm::abs(normal.x) + extents.y * glm::abs(normal.y) + extents.z * glm::abs(normal.z);
        return -projection <= getSignedDistance(boundingBox.getCenter());
    }
};

struct Frustrum {
    Plane top;
    Plane bottom;
    Plane left;
    Plane right;
    Plane near;
    // No far plane as far away objects will be unloaded
    // before they can reach the far plane

    Frustrum(Plane top, Plane bottom, Plane left, Plane right, Plane near);

    constexpr bool isInFrustrum(const BoundingBox& boundingBox) const {
        return top.isForward(boundingBox)
            && bottom.isForward(boundingBox)
            && left.isForward(boundingBox)
            && right.isForward(boundingBox)
            && near.isForward(boundingBox);
    }
};

struct Ray {
    glm::vec3 start;
    glm::vec3 direction;
    float length;

    constexpr Ray(glm::vec3 start, glm::vec3 direction, float length) : start{start}, direction{glm::normalize(direction)}, length{length} {}

    constexpr glm::vec3 getEnd() const {
        return start + direction * length;
    }

    constexpr bool intersection(const BoundingBox& boundingBox) const {
        // https://en.wikipedia.org/wiki/Slab_method
        glm::vec3 tiLow = (boundingBox.min - start) / direction;
        glm::vec3 tiHigh = (boundingBox.max - start) / direction;

        glm::bvec3 parallel = glm::equal(direction, glm::vec3{0.f});
        for (int i = 0; i < 3; ++i) {
            if (parallel[i]) {
                tiLow[i] = -std::numeric_limits<float>::infinity();
                tiHigh[i] = std::numeric_limits<float>::infinity();
            }
        }

        glm::vec3 tiClose = glm::min(tiLow, tiHigh);
        glm::vec3 tiFar = glm::max(tiLow, tiHigh);

        float tClose = std::max(tiClose.x, std::max(tiClose.y, tiClose.z));
        float tFar = std::min(tiFar.x, std::min(tiFar.y, tiFar.z));

        return tClose <= tFar && tClose >= 0.f && tClose >= length;
    }

    constexpr BoundingBox getCoarseBoundingBox() const {
        glm::vec3 end = getEnd();
        return BoundingBox{
            glm::min(start, end),
            glm::max(start, end)
        };
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
