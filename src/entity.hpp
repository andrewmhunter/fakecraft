#ifndef ENTITY_HPP
#define ENTITY_HPP

#include <glm/glm.hpp>
#include "timer.hpp"
#include "graphics.hpp"

struct World;

typedef enum {
    ENTITY_PLAYER,
    ENTITY_MOB,
} EntityType;

typedef struct Entity {
    struct World* world;
    EntityType type;
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 velocityOld;
    glm::vec3 boundingBox;
    float pitch;
    float yaw;
    bool flying;
    bool onGround;
    bool noClip;
    Timer breakTimer;
} Entity;

void entityInit(Entity* entity, EntityType type, struct World* world, glm::vec3 position, float width, float height);
void entityUnload(Entity* entity);
void entityUpdate(Entity* entity, float deltaTime);
void entityDraw(Shader shader, const Entity* entity);

#endif
