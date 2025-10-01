#include "entity.h"
#include "collision.h"
#include "logger.h"

void entityInit(Entity* entity, EntityType type, World* world, Vector3 position, float width, float height) {
    ASSERT(entity);
    ASSERT(world);

    entity->type = type;
    entity->world = world;
    entity->position = position;
    entity->velocity = Vector3Zero();
    entity->velocityOld = Vector3Zero();
    entity->boundingBox = (Vector3){width, height, width};
    entity->pitch = 0.f;
    entity->yaw = 0.f;
    entity->flying = false;
    entity->onGround = false;
    entity->noClip = false;
}

void entityUnload(Entity* entity) {
    ASSERT(entity);
    free(entity);
}



void playerUpdate(Entity* entity, float deltaTime) {
    if (IsKeyPressed(KEY_F)) {
        entity->flying = !entity->flying;
    }

    if (IsKeyPressed(KEY_N)) {
        entity->noClip = !entity->noClip;
    }

    Vector3 movement = Vector3Zero();

    if (IsKeyDown(KEY_W)) {
        movement.x += 1.f;
    }

    if (IsKeyDown(KEY_S)) {
        movement.x -= 1.f;
    }

    if (IsKeyDown(KEY_A)) {
        movement.z -= 1.f;
    }

    if (IsKeyDown(KEY_D)) {
        movement.z += 1.f;
    }

    if (entity->flying && IsKeyDown(KEY_LEFT_CONTROL)) {
        movement.y -= 1.f;
    }

    if (entity->flying && IsKeyDown(KEY_SPACE)) {
        movement.y += 1.f;
    }

    //float speed = entity->flying ? 10.79f : 4.317;
    //speed = entity->flying ? 21.58f : 5.612;

    float speed = 0.098f * deltaTime;

    if (IsKeyDown(KEY_LEFT_SHIFT)) {
        speed *= 2.f;
    }

    if (entity->flying) {
        speed *= 4.f;
    }

    //float speed = 0.2f * delta;


    Vector2 mouseDelta = GetMouseDelta();
    entity->yaw -= mouseDelta.x * SENSITIVITY;
    entity->pitch-= mouseDelta.y * SENSITIVITY;
    entity->pitch = Clamp(entity->pitch, -PI / 2 + 0.0001, PI / 2 - 0.0001);

    movement = Vector3RotateByAxisAngle(movement, (Vector3){0.f, 1.f, 0.f}, entity->yaw);
    movement = Vector3Normalize(movement);
    movement = Vector3Scale(movement, speed);

    if (!entity->flying && IsKeyDown(KEY_SPACE) && entity->onGround) {
        entity->velocity.y += 0.5f;
    }

    entity->velocity = Vector3Add(entity->velocity, movement);
}

//#define UPDATE_POSITION_FIRST

void updatePosition(Entity* entity, float deltaTime) {
    Vector3 avgVelocity = Vector3Scale(Vector3Add(entity->velocity, entity->velocityOld), deltaTime * 0.5f);
    //Vector3 avgVelocity = Vector3Scale(entity->velocity, );
    Vector3 baseVelocity = avgVelocity;

    if (!entity->noClip) {
        avgVelocity = aabbResolveCollisions(entity->world, entity->position, entity->boundingBox, avgVelocity);
    }
    //printf("%s\n", formatVector3(Vector3Scale(avgVelocity, GetFPS())));

    entity->position = Vector3Add(entity->position, avgVelocity);

    if (avgVelocity.x != baseVelocity.x) {
        entity->velocity.x = 0.f;
    }

    entity->onGround = false;
    if (avgVelocity.y != baseVelocity.y) {
        entity->velocity.y = 0.f;
        entity->onGround = baseVelocity.y < 0.f;
    }

    if (avgVelocity.z != baseVelocity.z) {
        entity->velocity.z = 0.f;
    }


    entity->velocityOld = entity->velocity;
}

void entityUpdate(Entity* entity, float deltaTime) {
    ASSERT(entity);

    if (entity->type == ENTITY_PLAYER) {
        playerUpdate(entity, deltaTime);
    }

    // Minecraft entity physics: https://minecraft.wiki/w/Entity#Motion
    // https://gamedev.stackexchange.com/questions/169558/how-can-i-fix-my-velocity-damping-to-work-with-any-delta-frame-time

#ifdef UPDATE_POSITION_FIRST
    updatePosition(entity, deltaTime);
#endif

    float horizontalDrag = 0.546f;
    //float horizontalDrag = 0.91f;
    float verticalDrag = 0.98f;


    if (!entity->flying) {
        entity->velocity.y -= (double)0.08 * deltaTime;
    }

    horizontalDrag = 1.f - powf(horizontalDrag, deltaTime);
    if (!entity->flying) {
        verticalDrag = 1.f - powf(verticalDrag, deltaTime);
    } else {
        verticalDrag = horizontalDrag;
    }

    entity->velocity.x = Lerp(entity->velocity.x, 0.f, horizontalDrag);
    entity->velocity.y = Lerp(entity->velocity.y, 0.f, verticalDrag);//* deltaTime * 20;
    entity->velocity.z = Lerp(entity->velocity.z, 0.f, horizontalDrag);

#ifndef UPDATE_POSITION_FIRST
    updatePosition(entity, deltaTime);
#endif
}

void drawPlayerModel(Vector3 position) {
    float unit = 1.f / 16.f;
    float bodyHeight = 12.f * unit;
    float limbWidth = 4.f * unit;
    float bodyWidth = 8.f * unit;

    Vector3 bodyPosition = {0.f, bodyHeight + bodyHeight / 2.f, 0.f};
    bodyPosition = Vector3Add(bodyPosition, position);

    DrawCube(bodyPosition, bodyWidth, bodyHeight, limbWidth, RED);

    float headSize = 8.f * unit;
    Vector3 headPosition = {0.f, 2.f * bodyHeight + headSize / 2.f, 0.f};
    headPosition = Vector3Add(headPosition, position);
    DrawCube(headPosition, headSize, headSize, headSize, BLUE);

    Vector3 legPosition = {limbWidth / 2.f, bodyHeight / 2.f, 0.f};
    Vector3 leftLegPosition = Vector3Add(legPosition, position);
    DrawCube(leftLegPosition, limbWidth, bodyHeight, limbWidth, GREEN);
    legPosition.x *= -1;
    Vector3 rightLegPosition = Vector3Add(legPosition, position);
    DrawCube(rightLegPosition, limbWidth, bodyHeight, limbWidth, PURPLE);

    Vector3 armPosition = {limbWidth / 2.f + bodyWidth / 2.f, bodyHeight + bodyHeight / 2.f, 0.f};
    Vector3 leftArmPosition = Vector3Add(armPosition, position);
    DrawCube(leftArmPosition, limbWidth, bodyHeight, limbWidth, GREEN);
    armPosition.x *= -1;
    Vector3 rightArmPosition = Vector3Add(armPosition, position);
    DrawCube(rightArmPosition, limbWidth, bodyHeight, limbWidth, PURPLE);
}

void entityDraw(const Entity* entity) {
    ASSERT(entity);

    if (entity->type != ENTITY_PLAYER) {
        drawPlayerModel(entity->position);

        DrawBoundingBox(genBoundingBox(entity->position, entity->boundingBox), WHITE);
    }
}

