#include <cmath>
#include "entity.hpp"
#include "collision.hpp"
#include "graphics.hpp"
#include "logger.hpp"
#include "input.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

Entity::Entity(World* world, glm::vec3 position, glm::vec3 boundingBox)
    : world{world},
    type{ENTITY_PLAYER},
    position{position},
    boundingBox{boundingBox},
    pitch{0.f},
    yaw{0.f}
{
}

Entity::~Entity() {

}

//#define UPDATE_POSITION_FIRST

void Entity::updatePosition(float deltaTime) {
    glm::vec3 avgVelocity = (velocity + velocityOld) * deltaTime * 0.5f;
    glm::vec3 baseVelocity = avgVelocity;

    if (!noClip) {
        avgVelocity = aabbResolveCollisions(world, position, boundingBox, avgVelocity);
    }

    position = position + avgVelocity;

    if (avgVelocity.x != baseVelocity.x) {
        velocity.x = 0.f;
    }

    onGround = false;
    if (avgVelocity.y != baseVelocity.y) {
        velocity.y = 0.f;
        onGround = baseVelocity.y < 0.f;
    }

    if (avgVelocity.z != baseVelocity.z) {
        velocity.z = 0.f;
    }


    velocityOld = velocity;
}

void Entity::update(float deltaTime) {
    // Minecraft entity physics: https://minecraft.wiki/w/Entity#Motion
    // https://gamedev.stackexchange.com/questions/169558/how-can-i-fix-my-velocity-damping-to-work-with-any-delta-frame-time

#ifdef UPDATE_POSITION_FIRST
    updatePosition(this, deltaTime);
#endif

    float horizontalDrag = 0.546f;
    //float horizontalDrag = 0.91f;
    float verticalDrag = 0.98f;


    if (!flying) {
        velocity.y -= (double)0.08 * deltaTime;
    }

    horizontalDrag = 1.f - powf(horizontalDrag, deltaTime);
    if (!flying) {
        verticalDrag = 1.f - powf(verticalDrag, deltaTime);
    } else {
        verticalDrag = horizontalDrag;
    }

    velocity.x = std::lerp(velocity.x, 0.f, horizontalDrag);
    velocity.y = std::lerp(velocity.y, 0.f, verticalDrag);//* deltaTime * 20;
    velocity.z = std::lerp(velocity.z, 0.f, horizontalDrag);

#ifndef UPDATE_POSITION_FIRST
    updatePosition(deltaTime);
#endif
}

void drawPlayerModel(ShaderProgram& shader, glm::vec3 position);

void Entity::draw(ShaderProgram& shader) {
    drawPlayerModel(shader, position);
    /*if (type != ENTITY_PLAYER) {

        //DrawBoundingBox(genBoundingBox(entity->position, entity->boundingBox), WHITE);
    }*/
}


Player::Player(World* world, glm::vec3 position)
    : Entity{world, position, glm::vec3{0.6f, 1.8f, 0.6f}}
{
}

void Player::update(float deltaTime) {
    Entity::update(deltaTime);

    if (keyPressed(GLFW_KEY_F)) {
        flying = !flying;
    }

    if (keyPressed(GLFW_KEY_N)) {
        noClip = !noClip;
    }

    glm::vec3 movement{0.f};

    if (keyDown(GLFW_KEY_W)) {
        movement.x += 1.f;
    }

    if (keyDown(GLFW_KEY_S)) {
        movement.x -= 1.f;
    }

    if (keyDown(GLFW_KEY_A)) {
        movement.z -= 1.f;
    }

    if (keyDown(GLFW_KEY_D)) {
        movement.z += 1.f;
    }

    if (flying && keyDown(GLFW_KEY_LEFT_CONTROL)) {
        movement.y -= 1.f;
    }

    if (flying && keyDown(GLFW_KEY_SPACE)) {
        movement.y += 1.f;
    }

    //float speed = flying ? 10.79f : 4.317;
    //speed = flying ? 21.58f : 5.612;

    //float speed = 0.098f * deltaTime;
    float speed = 0.15f * deltaTime;

    if (keyDown(GLFW_KEY_LEFT_SHIFT)) {
        speed *= 2.f;
    }

    if (flying) {
        speed *= 4.f;
    }

    glm::dvec2 mouseDelta = getMouseDelta() * SENSITIVITY;
    yaw -= mouseDelta.x;
    pitch -= mouseDelta.y;
    pitch = glm::clamp(pitch, -glm::pi<float>() / 2.f + 0.01f, glm::pi<float>() / 2.f - 0.01f);

    movement = glm::vec3{glm::rotate(glm::mat4{1.f}, yaw, {0.f, 1.f, 0.f}) * glm::vec4{movement, 1.f}};
    if (movement != glm::vec3{0.f}) {
        movement = glm::normalize(movement);
    }

    movement *= speed;

    if (!flying && keyDown(GLFW_KEY_SPACE) && onGround) {
        velocity.y += 0.5f;
    }

    velocity += movement;
}

void Player::draw(ShaderProgram& shader) {
    (void)shader;
    //Entity::draw(shader);
}

class EntityModelState {
public:
    float modelYaw;
    float headYaw;
    float headPitch;

    float leftArmPitch;
    float rightArmPitch;
    float leftLegPitch;
    float rightLegPitch;

    EntityModelState(
        float modelYaw,
        float headYaw,
        float headPitch,
        float leftArmPitch,
        float rightArmPitch,
        float leftLegPitch,
        float rightLegPitch
    );
};

EntityModelState::EntityModelState(
    float modelYaw,
    float headYaw,
    float headPitch,
    float leftArmPitch,
    float rightArmPitch,
    float leftLegPitch,
    float rightLegPitch
) :
    modelYaw{modelYaw},
    headYaw{headYaw},
    headPitch{headPitch},
    leftArmPitch{leftArmPitch},
    rightArmPitch{rightArmPitch},
    leftLegPitch{leftLegPitch},
    rightLegPitch{rightLegPitch}
{}

class EntityModelPart {
public:
    glm::vec3 size;
    glm::vec3 origin;
    glm::vec3 attatchPoint;

    EntityModelPart(glm::vec3 size, glm::vec3 origin, glm::vec3 attatchPoint);
    glm::mat4 applyTransform(glm::mat4 transform, float yaw) const;
};

EntityModelPart::EntityModelPart(
    glm::vec3 size,
    glm::vec3 origin,
    glm::vec3 attatchPoint
) :
    size{size},
    origin{origin},
    attatchPoint{attatchPoint}
{}

glm::mat4 EntityModelPart::applyTransform(glm::mat4 transform, float yaw) const {
    transform = glm::translate(transform, attatchPoint);
    transform = glm::rotate(transform, yaw, {1.f, 0.f, 0.f});
    transform = glm::scale(transform, size);
    transform = glm::translate(transform, -origin);
    return transform;
}


class EntityModel {
public:
    void draw(ShaderProgram& shader, EntityModelState state);
};

void drawPlayerModel(ShaderProgram& shader, glm::vec3 position) {
    float unit = 1.f / 16.f;
    float bodyHeight = 12.f * unit;
    float limbWidth = 4.f * unit;
    float bodyWidth = 8.f * unit;

    glm::vec3 bodyPosition = {0.f, bodyHeight + bodyHeight / 2.f, 0.f};
    bodyPosition = bodyPosition + position;

    shader.setUniformVec4("color", color::red);
    drawCube(shader, bodyPosition, {bodyWidth, bodyHeight, limbWidth});

    float headSize = 8.f * unit;
    glm::vec3 headPosition = {0.f, 2.f * bodyHeight + headSize / 2.f, 0.f};
    headPosition = headPosition + position;
    shader.setUniformVec4("color", color::blue);
    drawCube(shader, headPosition, {headSize, headSize, headSize});

    glm::vec3 legPosition = {limbWidth / 2.f, bodyHeight / 2.f, 0.f};
    glm::vec3 leftLegPosition = legPosition + position;
    shader.setUniformVec4("color", color::green);
    drawCube(shader, leftLegPosition, {limbWidth, bodyHeight, limbWidth});
    legPosition.x *= -1;
    glm::vec3 rightLegPosition = legPosition + position;
    shader.setUniformVec4("color", color::magenta);
    drawCube(shader, rightLegPosition, {limbWidth, bodyHeight, limbWidth});

    glm::vec3 armPosition = {limbWidth / 2.f + bodyWidth / 2.f, bodyHeight + bodyHeight / 2.f, 0.f};
    glm::vec3 leftArmPosition = armPosition + position;
    shader.setUniformVec4("color", color::green);
    drawCube(shader, leftArmPosition, {limbWidth, bodyHeight, limbWidth});
    armPosition.x *= -1;
    glm::vec3 rightArmPosition = armPosition + position;
    shader.setUniformVec4("color", color::magenta);
    drawCube(shader, rightArmPosition, {limbWidth, bodyHeight, limbWidth});
}

void entityDraw(ShaderProgram& shader, const Entity* entity) {
    Logger::assertion(entity);

    if (entity->type != ENTITY_PLAYER) {
        drawPlayerModel(shader, entity->position);

        //DrawBoundingBox(genBoundingBox(entity->position, entity->boundingBox), WHITE);
    }
}

