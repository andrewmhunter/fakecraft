#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include "level/collision.hpp"

constexpr glm::vec3 worldUp{0.f, 1.f, 0.f};

class Camera {
public:
    static glm::ivec2 windowSize;

    virtual glm::mat4 getViewMatrix() const = 0;
    virtual glm::mat4 getProjectionMatrix() const = 0;

    virtual void use() const;
};

class PerspectiveCamera : public Camera {
public:
    glm::vec3 position;
    glm::vec3 lookDirection;
    float fov;

    float nearClippingPlane = 0.1f;
    float farClippingPlane = 1000.f; 

    float getAspectRatio() const;

    glm::vec3 getUp() const;
    glm::vec3 getDown() const;
    glm::vec3 getRight() const;
    glm::vec3 getLeft() const;

    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::mat4 getProjectionMatrix() const override;

    Frustrum getFrustrum() const;

    virtual void use() const override;
};

extern PerspectiveCamera globalCamera;



class OrthoCamera : public Camera {
public:
    glm::mat4 view;

    explicit OrthoCamera(glm::mat4 view);

    virtual glm::mat4 getViewMatrix() const override;
    virtual glm::mat4 getProjectionMatrix() const override;
};

#endif
