#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <compare>
#include <cstdint>

#include <glm/glm.hpp>
#include "level/collision.hpp"
#include "util/timer.hpp"
#include "graphics/graphics.hpp"

class World;

struct EntityID {
    std::uint64_t id;

    explicit EntityID(std::uint64_t id);

    std::strong_ordering operator<=>(const EntityID& other) const = default;

    EntityID& operator++();
    EntityID operator++(int);
};

enum class EntityType {
    player,
    mob,
};

class Entity {
private:
    void updatePosition(float deltaTime);

public:
    const EntityID id;
    World* world;
    EntityType type;
    glm::vec3 position;
    glm::vec3 velocity{0.f};
    glm::vec3 velocityOld{0.f};
    glm::vec3 size;
    float pitch{0.f};
    float yaw{0.f};
    float bodyYaw{0.f};
    bool onGround{false};
    bool noClip{false};
    bool flying{false};
    
    explicit Entity(World* world, EntityID id, glm::vec3 position, glm::vec3 size);
    virtual ~Entity();
    
    virtual void update(float deltaTime);
    virtual void draw(ShaderProgram& shader);
    virtual void collide(float deltaTime, EntityID otherID);

    virtual BoundingBox getBoundingBox() const;
};

class Human : public Entity {
public:
    float armRotation{0.f};

    explicit Human(World* world, EntityID id, glm::vec3 position);

    virtual void update(float deltaTime) override;
    virtual void draw(ShaderProgram& shader) override;
};

class Player final : public Entity {
public:
    Timer breakTimer{timerInit(0.25)};

    explicit Player(World* world, EntityID id, glm::vec3 position);

    virtual void update(float deltaTime) override;
    virtual void draw(ShaderProgram& shader) override;
};

#endif
