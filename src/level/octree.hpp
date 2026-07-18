#ifndef OCTREE_HPP
#define OCTREE_HPP

#include "collision.hpp"
#include "entities/entity.hpp"
#include "util/point.hpp"
#include "util/point.hpp"
#include <glm/fwd.hpp>
#include <map>
#include <memory>
#include <array>
#include <optional>
#include <set>

#define TRIVIAL_COLLISIONS 0

class Octree {
private:
    bool spilled{false};
    std::array<std::unique_ptr<Octree>, 8> partitions{nullptr};
    std::map<EntityID, BoundingBox> leaves{};

    std::optional<std::pair<int, glm::ivec3>> getChildIndex(const BoundingBox& boundingBox) const;
    bool spillEntity(EntityID entityID, const BoundingBox& boundingBox);

public:
    static constexpr int maximumDepth = 5;
    static constexpr int maximumLeaves = 8;

    const int depth;
    const glm::ivec3 position;
    const glm::ivec3 size;
    const glm::vec3 center;

    explicit Octree(int depth, glm::ivec3 position, glm::ivec3 size);

    void insertEntity(EntityID entityID, BoundingBox boundingBox);
    void addCollisions(std::set<EntityID>& collisions, const BoundingBox& boundingBox) const;
};

class CollisionWorld {
private:
    std::map<glm::ivec3, std::unique_ptr<Octree>, CompareIvec3FO> sections;

#if TRIVIAL_COLLISIONS
    std::map<EntityID, BoundingBox> contents;
#endif

    std::set<glm::ivec3, CompareIvec3FO> getOctreePositions(const BoundingBox& boundingBox) const;

public:
    static constexpr glm::ivec3 collisionSectionSize{1 << Octree::maximumDepth};

    void insertEntity(EntityID entityID, const BoundingBox& boundingBox);
    std::set<EntityID> getCollisions(const BoundingBox& boundingBox, EntityID remove) const;
};

#endif
