#ifndef ENTITY_MODEL_HPP
#define ENTITY_MODEL_HPP

#include "graphics/graphics.hpp"
#include <glm/fwd.hpp>
#include <span>

template<typename T>
using TextureCoords = std::pair<glm::vec<2, T>, glm::vec<2, T>>;

class EntityModelPart {
private:
    GPUMesh mesh;

    static Mesh generateMesh(glm::vec3 origin, glm::vec3 size, glm::ivec2 textureSize,
        std::span<const TextureCoords<int>, 6> texCoords
    );

public:

    explicit EntityModelPart(glm::vec3 origin, glm::vec3 size, glm::ivec2 textureSize,
        std::span<const TextureCoords<int>, 6> texCoords
    );

    explicit EntityModelPart(glm::vec3 origin, glm::ivec3 sizePixels, glm::ivec2 textureSize, glm::ivec2 netTexcoord);

    void draw(ShaderProgram& shader, glm::mat4 transform) const;
};

class HumanModelState {
public:
    float yaw;
    float bodyYaw;
    float pitch;
    float limbRotation;

    explicit HumanModelState(float yaw, float bodyYaw, float pitch, float limbRotation);
};

class HumanModel {
private:
    EntityModelPart head;
    EntityModelPart torso;
    EntityModelPart armLeft;
    EntityModelPart armRight;
    EntityModelPart leg;

public:
    explicit HumanModel();

    void draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const;
};

class PigModel {
private:
    EntityModelPart head;
    EntityModelPart torso;
    EntityModelPart leg;

public:
    explicit PigModel();
    
    void draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const;
};

#endif