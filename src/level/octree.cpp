#include "octree.hpp"
#include "engine/logger.hpp"
#include "entities/entity.hpp"
#include "level/collision.hpp"
#include "util/point.hpp"
#include <format>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/vector_relational.hpp>
#include <memory>


Octree::Octree(int depth, glm::ivec3 position, glm::ivec3 size)
    : depth{depth},
    position{position},
    size{size},
    center{position + size / 2}
{}

std::optional<std::pair<int, glm::ivec3>> Octree::getChildIndex(const BoundingBox& boundingBox) const {
    glm::bvec3 greater = glm::greaterThan(boundingBox.min, center);
    glm::bvec3 less = glm::lessThan(boundingBox.max, center);

    if ((greater || less) != glm::bvec3{true}) {
        return std::nullopt;
    }
    
    glm::ivec3 octPosition = glm::ivec3{greater};
    int octIndex = octPosition.x + octPosition.y * 2 + octPosition.z * 4;
    return std::pair{octIndex, octPosition};
}

bool Octree::spillEntity(EntityID entityID, const BoundingBox& boundingBox) {
    auto octIndex = getChildIndex(boundingBox);

    if (!octIndex.has_value()) {


        return false;
    }

    auto [index, octPosition] = octIndex.value();
        
    if (partitions[index] == nullptr) {
        glm::ivec3 newPosition = position + octPosition * (size / 2);
        partitions[index] = std::make_unique<Octree>(depth + 1, newPosition, size / 2);
        Logger::trace(std::format("New octree created. Depth: {}", depth + 1));
    }

    partitions[index]->insertEntity(entityID, boundingBox);
    return true;
}

void Octree::insertEntity(EntityID entityID, BoundingBox boundingBox) {
    if (spilled && spillEntity(entityID, boundingBox)) {
        return;
    }

    leaves.emplace(entityID, boundingBox);
    if (leaves.size() > maximumLeaves && depth < maximumDepth) {
        spilled = true;
        for (auto& entity : leaves) {
            spillEntity(entity.first, entity.second);
        }
    }
}

void Octree::addCollisions(std::set<EntityID>& collisions, const BoundingBox& boundingBox) const {
    auto octIndex = getChildIndex(boundingBox);
    
    for (const auto& leaf : leaves) {
        if (boundingBox.intersects(leaf.second)) {
            collisions.insert(leaf.first);
        }
    }
    
    if (octIndex.has_value()) {
        const auto& partition = partitions[octIndex.value().first];
        if (partition == nullptr) {
            return;
        }
        partition->addCollisions(collisions, boundingBox);
        return;
    }

    for (const auto& partition : partitions) {
        if (partition != nullptr) {
            partition->addCollisions(collisions, boundingBox);
        }
    }
}

static float boundingBoxMinOrMax(const BoundingBox& boundingBox, int axis, bool getMax) {
    return (getMax ? boundingBox.max : boundingBox.min)[axis];
}

std::set<glm::ivec3, CompareIvec3FO> CollisionWorld::getOctreePositions(const BoundingBox& boundingBox) const {
    std::set<glm::ivec3, CompareIvec3FO> octrees;
    for (int xi = false; xi <= 1; ++xi) {
        float x = boundingBoxMinOrMax(boundingBox, 0, xi);
        for (int yi = 0; yi <= 1; ++yi) {
            float y = boundingBoxMinOrMax(boundingBox, 1, yi);
            for (int zi = 0; zi <= 1; ++zi) {
                float z = boundingBoxMinOrMax(boundingBox, 2, zi);
                glm::vec3 position{x, y, z};

                position /= collisionSectionSize;
                glm::ivec3 octree = glm::floor(position);
                octrees.insert(octree);
            }
        }
    }

    return octrees;
}

void CollisionWorld::insertEntity(EntityID entityID, const BoundingBox& boundingBox) {
#if TRIVIAL_COLLISIONS
    contents.emplace(entityID, boundingBox);
    return;
#endif

    auto octreePositions = getOctreePositions(boundingBox);

    for (glm::ivec3 octreePosition : octreePositions) {
        if (!sections.contains(octreePosition)) {
            std::unique_ptr<Octree> newOctree = std::make_unique<Octree>(0, octreePosition * collisionSectionSize, collisionSectionSize);
            sections[octreePosition] = std::move(newOctree);
        }

        sections[octreePosition]->insertEntity(entityID, boundingBox);
    }
}


std::set<EntityID> CollisionWorld::getCollisions(const BoundingBox& boundingBox, EntityID remove) const {
    std::set<EntityID> collisions;

#if TRIVIAL_COLLISIONS
    for (const auto& entity : contents) {
        if (boundingBox.intersects(entity.second)) {
            collisions.insert(entity.first);
        }
    }
    return collisions;
#endif

    auto octreePositions = getOctreePositions(boundingBox);

    for (glm::ivec3 octreePosition : octreePositions) {
        if (sections.contains(octreePosition)) {
            sections.at(octreePosition)->addCollisions(collisions, boundingBox);
        }
    }


    collisions.erase(remove);
    return collisions;
}
