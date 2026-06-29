#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <glm/glm.hpp>
#include "timer.hpp"
#include "graphics.hpp"

class World;

typedef enum {
    ENTITY_PLAYER,
    ENTITY_MOB,
} EntityType;

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
    float pitch;
    float yaw;
    bool onGround{false};
    bool noClip{false};
    bool flying{false};
    
    Entity(World* world, glm::vec3 position, glm::vec3 boundingBox);
    virtual ~Entity();
    
    virtual void update(float deltaTime);
    virtual void draw(ShaderProgram& shader);
};

class Human : public Entity {
public:
};

class Player : public Entity {
public:
    Timer breakTimer{timerInit(0.25)};

    Player(World* world, glm::vec3 position);

    virtual void update(float deltaTime) override;
    virtual void draw(ShaderProgram& shader) override;
};

void entityInit(Entity* entity, EntityType type, World* world, glm::vec3 position, float width, float height);
void entityUnload(Entity* entity);
void entityUpdate(Entity* entity, float deltaTime);
void entityDraw(ShaderProgram& shader, const Entity* entity);

#endif
