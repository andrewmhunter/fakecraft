#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <glm/glm.hpp>
#include "timer.hpp"
#include "graphics.hpp"

class World;

enum class EntityType {
    player,
    mob,
};

class Entity {
private:
    void updatePosition(float deltaTime);

public:
    World* world;
    EntityType type;
    glm::vec3 position;
    glm::vec3 velocity{0.f};
    glm::vec3 velocityOld{0.f};
    glm::vec3 boundingBox;
    float pitch{0.f};
    float yaw{0.f};
    float bodyYaw{0.f};
    bool onGround{false};
    bool noClip{false};
    bool flying{false};
    
    explicit Entity(World* world, glm::vec3 position, glm::vec3 boundingBox);
    virtual ~Entity();
    
    virtual void update(float deltaTime);
    virtual void draw(ShaderProgram& shader);
};

class Human : public Entity {
public:
    explicit Human(World* world, glm::vec3 position);

    virtual void update(float deltaTime) override;
    virtual void draw(ShaderProgram& shader) override;
};

class Player final : public Entity {
public:
    Timer breakTimer{timerInit(0.25)};

    explicit Player(World* world, glm::vec3 position);

    virtual void update(float deltaTime) override;
    virtual void draw(ShaderProgram& shader) override;
};

#endif
