#include <cmath>

#include <cstdint>
#include <glm/geometric.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>

#include "entities/entity.hpp"
#include "entities/entity_model.hpp"
#include "level/collision.hpp"
#include "level/world.hpp"
#include "graphics/graphics.hpp"
#include "engine/input.hpp"
#include "engine/resource_manager.hpp"

EntityID::EntityID(std::uint64_t id)
    : id{id}
{}

EntityID& EntityID::operator++() {
    ++id;
    return *this;
}

EntityID EntityID::operator++(int) {
    EntityID old = *this;
    ++*this;
    return old;
}


Entity::Entity(World* world, EntityID id, glm::vec3 position, glm::vec3 boundingBox)
    : id{id},
    world{world},
    type{EntityType::player},
    position{position},
    size{boundingBox}
{}

Entity::~Entity() {}


void Entity::updatePosition(float deltaTime) {
    glm::vec3 avgVelocity = (velocity + velocityOld) * deltaTime * 0.5f;
    glm::vec3 baseVelocity = avgVelocity;

    if (!noClip) {
        avgVelocity = aabbResolveCollisions(world, position, size, avgVelocity);
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

constexpr float force = 0.2f;

void Entity::collide(float deltaTime, EntityID otherID) {
    const Entity& other = *world->entities[otherID];

    glm::vec3 pushForce = position - other.position;
    pushForce.y = 0.f;
    velocity += pushForce * force * deltaTime;
}

BoundingBox Entity::getBoundingBox() const {
    glm::vec3 sides{size.x / 2, 0.f, size.z / 2.f};
    glm::vec3 min = position - sides;
    sides.y = size.y;
    glm::vec3 max = position + sides;
    return BoundingBox{min, max};
}

void drawPlayerModel(ShaderProgram& shader, glm::vec3 position);

void Entity::draw(ShaderProgram& shader) {
    (void)shader;
}

Human::Human(World* world, EntityID id, glm::vec3 position)
: Entity{world, id, position, glm::vec3{0.6f, 1.8f, 0.6f}}
{}

void Human::update(float deltaTime) {
    Entity::update(deltaTime);

    armRotation += deltaTime;

    if (glm::distance(position, world->player->position) < 15.f) {
        glm::vec3 lookVector = world->player->position - position;
        yaw = glm::atan(lookVector.x, lookVector.z);
        pitch = glm::atan(-lookVector.y, glm::length(glm::vec2{lookVector.x, lookVector.z}));

        velocity += glm::normalize(lookVector) * deltaTime * 0.1f;
        if (onGround) {
            velocity.y += 0.5f;
        }
    }
}

void Human::draw(ShaderProgram& shader) {
    Entity::draw(shader);
    ResourceManager::instance().entityModel.human.draw(shader, position, HumanModelState{yaw, bodyYaw, pitch, std::sin(armRotation / 2.f)});
}


Player::Player(World* world, EntityID id, glm::vec3 position)
    : Entity{world, id, position, glm::vec3{0.6f, 1.8f, 0.6f}}
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
