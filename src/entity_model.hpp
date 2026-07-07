#ifndef ENTITY_MODEL_HPP
#define ENTITY_MODEL_HPP

#include "graphics.hpp"
#include "util.hpp"
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

    void draw(ShaderProgram& shader, glm::mat4 transform) const;
};

class HumanModelState {

};

class HumanModel {
private:
    EntityModelPart head;
    EntityModelPart torso;
    EntityModelPart armLeft;
    EntityModelPart armRight;
    EntityModelPart leg;

public:
    HumanModel();

    void draw(ShaderProgram& shader, glm::vec3 position) const;
};

#endif