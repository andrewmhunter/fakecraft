#include "camera.hpp"
#include "graphics/graphics.hpp"
#include "level/collision.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

PerspectiveCamera globalCamera;

glm::ivec2 Camera::windowSize{800.f, 600.f};

void Camera::use() const {
    setProjectionView(getProjectionMatrix() * getViewMatrix());
}

glm::vec3 PerspectiveCamera::getUp() const {
    return -glm::normalize(glm::cross(lookDirection, getRight()));
}

glm::vec3 PerspectiveCamera::getDown() const {
    return getUp();
}

glm::vec3 PerspectiveCamera::getRight() const {
    return glm::normalize(glm::cross(worldUp, lookDirection));
}

glm::vec3 PerspectiveCamera::getLeft() const {
    return -getRight();
}

float PerspectiveCamera::getAspectRatio() const {
    return static_cast<float>(windowSize.x) / static_cast<float>(windowSize.y);
}

glm::mat4 PerspectiveCamera::getViewMatrix() const {
    return glm::lookAt(position, position + lookDirection, worldUp);
}

glm::mat4 PerspectiveCamera::getProjectionMatrix() const {
    return glm::perspective(
        glm::radians(fov),
        getAspectRatio(),
        nearClippingPlane,
        farClippingPlane
    );
}

Frustrum PerspectiveCamera::getFrustrum() const {
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

void PerspectiveCamera::use() const {
    setProjectionView(getProjectionMatrix() * getViewMatrix(), position);
}

OrthoCamera::OrthoCamera(glm::mat4 view) : view{view} {}

glm::mat4 OrthoCamera::getProjectionMatrix() const {
    return glm::ortho(
        -windowSize.x / 2.f, windowSize.x / 2.f,
        -windowSize.y / 2.f, windowSize.y / 2.f
    );
}


glm::mat4 OrthoCamera::getViewMatrix() const {
    return view;
}


