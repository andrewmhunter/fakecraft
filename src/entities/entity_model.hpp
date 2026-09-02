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

    void generateMesh(EntityMesh& mesh) const;

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

using Bones = std::array<glm::mat4, maxBones>;
using BoneSpan = std::span<glm::mat4, maxBones>;

using EntityDrawFeatures = std::pair<const Texture*, const GPUMesh*>;

using EntityDrawCommands = std::map<EntityDrawFeatures, std::vector<Bones>>;

class EntityModel {
private:
    static GLint bonesUniformLocation;

public:
    GPUMesh mesh;

    explicit EntityModel(std::span<const EntityModelPart> parts);

    virtual void getBones(BoneSpan bones, glm::vec3 position, const HumanModelState& state) const = 0;
    void draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const;
    void appendDrawCommands(EntityDrawCommands& commands, const Texture& texture, glm::vec3 position, const HumanModelState& state) const;
};

class HumanModel : public EntityModel {
private:
public:
    explicit HumanModel();

    virtual void getBones(BoneSpan bones, glm::vec3 position, const HumanModelState& state) const override;
};

class PigModel : public EntityModel {
private:
public:
    explicit PigModel();

    virtual void getBones(BoneSpan bones, glm::vec3 position, const HumanModelState& state) const override;
};

#endif