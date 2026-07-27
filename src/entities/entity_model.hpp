#ifndef ENTITY_MODEL_HPP
#define ENTITY_MODEL_HPP

#include "graphics/graphics.hpp"
#include <glm/fwd.hpp>
#include <span>

template<typename T>
using TextureCoords = std::pair<glm::vec<2, T>, glm::vec<2, T>>;

class EntityModelPart {
private:
    glm::vec3 origin;
    glm::vec3 size;
    glm::ivec2 textureSize;
    std::array<TextureCoords<int>, 6> texCoords;
    int boneId;

public:

    void generateMesh(Mesh& mesh) const;

    EntityModelPart(glm::vec3 origin, glm::vec3 size, glm::ivec2 textureSize,
        std::array<TextureCoords<int>, 6> texCoords, int boneId
    );

    EntityModelPart(glm::vec3 origin, glm::ivec3 sizePixels, glm::ivec2 textureSize, glm::ivec2 netTexcoord, int boneId);
};

class HumanModelState {
public:
    float yaw;
    float bodyYaw;
    float pitch;
    float limbRotation;

    explicit HumanModelState(float yaw, float bodyYaw, float pitch, float limbRotation);
};

class EntityModel {
private:
public:
    GPUMesh mesh;

    explicit EntityModel(std::span<const EntityModelPart> parts);

    virtual void getBones(std::span<glm::mat4, maxBones> bones, glm::vec3 position, const HumanModelState& state) const = 0;
    void draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const;
};

class HumanModel : public EntityModel {
private:
public:
    explicit HumanModel();

    virtual void getBones(std::span<glm::mat4, maxBones> bones, glm::vec3 position, const HumanModelState& state) const override;
};

class PigModel : public EntityModel {
private:
public:
    explicit PigModel();

    virtual void getBones(std::span<glm::mat4, maxBones> bones, glm::vec3 position, const HumanModelState& state) const override;
};

#endif