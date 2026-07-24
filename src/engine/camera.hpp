#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include "level/collision.hpp"

constexpr glm::vec3 worldUp{0.f, 1.f, 0.f};

class Camera {
public:
    static Camera globalCamera;

    glm::vec3 position;
    glm::vec3 lookDirection;
    float fov;

    glm::ivec2 windowSize{800.f, 600.f};

    float nearClippingPlane = 0.1f;
    float farClippingPlane = 1000.f;

    glm::vec3 getUp() const;
    glm::vec3 getDown() const;
    glm::vec3 getRight() const;
    glm::vec3 getLeft() const;

    float getAspectRatio() const;

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    glm::mat4 getProjectionViewMatrix() const;

    glm::mat4 getGUIProjection() const;

    Frustrum getFrustrum() const;
};

#endif
