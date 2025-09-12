#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>

struct World;

typedef enum {
    ENTITY_PLAYER,
    ENTITY_MOB,
} EntityType;

typedef struct {
    struct World* world;
    EntityType type;
    Vector3 position;
    Vector3 velocity;
    Vector3 velocityOld;
    Vector3 boundingBox;
    float pitch;
    float yaw;
} Entity;

void entityInit(Entity* entity, EntityType type, struct World* world, Vector3 position, float width, float height);
void entityUnload(Entity* entity);
void entityUpdate(Entity* entity, float deltaTime);
void entityDraw(const Entity* entity);

#endif
