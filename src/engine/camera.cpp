#include "camera.hpp"
#include "level/collision.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

Camera Camera::globalCamera{};

glm::vec3 Camera::getUp() const {
    return -glm::normalize(glm::cross(lookDirection, getRight()));
}

glm::vec3 Camera::getDown() const {
    return getUp();
}

glm::vec3 Camera::getRight() const {
    return glm::normalize(glm::cross(worldUp, lookDirection));
}

glm::vec3 Camera::getLeft() const {
    return -getRight();
}

float Camera::getAspectRatio() const {
    return static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + lookDirection, worldUp);
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(fov),
        getAspectRatio(),
        nearClippingPlane,
        farClippingPlane
    );
}

glm::mat4 Camera::getProjectionViewMatrix() const {
    return getProjectionMatrix() * getViewMatrix();
}

glm::mat4 Camera::getGUIProjection() const {
    return glm::ortho(
        -windowSize.x / 2.f, windowSize.x / 2.f,
        -windowSize.y / 2.f, windowSize.y / 2.f
    );
}

Frustrum Camera::getFrustrum() const {
    // https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    float radiansFov = glm::radians(fov);
    const float halfVSide = farClippingPlane * std::tanf(radiansFov * .5f);
    const float halfHSide = halfVSide * getAspectRatio();
    const glm::vec3 frontMultFar = farClippingPlane * lookDirection;

    Plane nearFace { position + nearClippingPlane * lookDirection, lookDirection };
    Plane farFace { position + frontMultFar, -lookDirection };
    Plane rightFace { position, glm::cross(frontMultFar - getRight() * halfHSide, getUp()) };
    Plane leftFace { position, glm::cross(getUp(), frontMultFar + getRight() * halfHSide) };
    Plane topFace { position, glm::cross(getRight(), frontMultFar - getUp() * halfVSide) };
    Plane bottomFace { position, glm::cross(frontMultFar + getUp() * halfVSide, getRight()) };

    return Frustrum{topFace, bottomFace, leftFace, rightFace, nearFace};
}
