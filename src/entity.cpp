#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "entity.hpp"
#include "collision.hpp"
#include "graphics.hpp"
#include "input.hpp"
#include "resource_manager.hpp"

Entity::Entity(World* world, glm::vec3 position, glm::vec3 boundingBox)
    : world{world},
    type{EntityType::player},
    position{position},
    boundingBox{boundingBox}
{
}

Entity::~Entity() {

}


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

constexpr float allowedNeckRotation = glm::radians(60.f);

void Entity::update(float deltaTime) {
    // Minecraft entity physics: https://minecraft.wiki/w/Entity#Motion
    // https://gamedev.stackexchange.com/questions/169558/how-can-i-fix-my-velocity-damping-to-work-with-any-delta-frame-time

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

    updatePosition(deltaTime);


    float yawDifference = yaw - bodyYaw;
    if (yawDifference < -glm::pi<float>()) {
        yawDifference += 2 * glm::pi<float>();
    } else if (yawDifference > glm::pi<float>()) {
        yawDifference -= 2 * glm::pi<float>();
    }

    if (yawDifference > allowedNeckRotation) {
        bodyYaw = yaw - allowedNeckRotation;
    } else if (yawDifference < -allowedNeckRotation) {
        bodyYaw = yaw + allowedNeckRotation;
    }
}

void drawPlayerModel(ShaderProgram& shader, glm::vec3 position);

void Entity::draw(ShaderProgram& shader) {
    (void)shader;
}

Human::Human(World* world, glm::vec3 position)
: Entity{world, position, glm::vec3{0.6f, 1.8f, 0.6f}}
{}

void Human::update(float deltaTime) {
    Entity::update(deltaTime);
    
    glm::vec3 lookVector = world->player->position - position;
    yaw = glm::atan(lookVector.x, lookVector.z);
    pitch = glm::atan(-lookVector.y, glm::length(glm::vec2{lookVector.x, lookVector.z}));
}

void Human::draw(ShaderProgram& shader) {
    Entity::draw(shader);
    ResourceManager::instance().entityModel.human.draw(shader, position, yaw, pitch, bodyYaw);
}


Player::Player(World* world, glm::vec3 position)
    : Entity{world, position, glm::vec3{0.6f, 1.8f, 0.6f}}
{}

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

    glm::dvec2 mouseDelta = getMouseDelta() * static_cast<double>(Config::settings->controls.sensitivity);
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
    Entity::draw(shader);
}
