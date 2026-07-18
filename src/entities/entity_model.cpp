#include "entity_model.hpp"
#include "util/direction.hpp"
#include "graphics/graphics.hpp"
#include <cstddef>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include "util/util.hpp"

Mesh EntityModelPart::generateMesh(glm::vec3 origin, glm::vec3 size, glm::ivec2 textureSize,
    std::span<const TextureCoords<int>, directionCount> texCoords
) {
    Mesh mesh{GL_TRIANGLES};

    glm::mat4 baseTransform{1.f};
    baseTransform = glm::scale(baseTransform, size);
    baseTransform = glm::translate(baseTransform, -origin);

    std::array<TextureCoords<float>, directionCount> adjustedTexCoords;
    for (std::size_t i = 0; i < texCoords.size(); ++i) {
        adjustedTexCoords[i].first = glm::vec2{texCoords[i].first} / glm::vec2{textureSize};
        adjustedTexCoords[i].second = glm::vec2{texCoords[i].second} / glm::vec2{textureSize};
    }

    mesh.pushTexturedPrism(baseTransform, adjustedTexCoords);
    return mesh;
}

EntityModelPart::EntityModelPart(glm::vec3 origin, glm::vec3 size, glm::ivec2 textureSize,
    std::span<const TextureCoords<int>, 6> texCoords
) : mesh{generateMesh(origin, size, textureSize, texCoords).upload()}
{}

void EntityModelPart::draw(ShaderProgram& shader, glm::mat4 transform) const {
    shader.setUniformMat4("model", transform);
    mesh.draw();
}

HumanModelState::HumanModelState(float yaw, float bodyYaw, float pitch, float limbRotation)
    : yaw{yaw}, bodyYaw{bodyYaw}, pitch{pitch}, limbRotation{limbRotation}
{}

HumanModel::HumanModel()
    : head{{0.f, -0.5f, 0.f}, glm::vec3{8._px}, {64, 32}, std::array<TextureCoords<int>, 6>{
        TextureCoords<int>
        {{8, 8}, {16, 16}},
        {{24, 8}, {32, 16}},
        {{16, 8}, {24, 16}},
        {{0, 8}, {8, 16}},
        {{8, 0}, {16, 8}},
        {{16, 0}, {24, 8}}}
    },
    torso{{0.f, -0.5f, 0.f}, {8._px, 12._px, 4._px}, {64, 32}, std::array<TextureCoords<int>, 6>{
        TextureCoords<int>
        {{20, 20}, {28, 32}},
        {{32, 20}, {40, 32}},
        {{28, 20}, {32, 32}},
        {{16, 20}, {20, 32}},
        {{20, 16}, {28, 20}},
        {{28, 16}, {36, 20}}}
    },
    armLeft{{-0.5f, 0.5f, 0.f}, {4._px, 12._px, 4._px}, {64, 32}, std::array<TextureCoords<int>, 6>{
        TextureCoords<int>
        {{44, 20}, {48, 32}},
        {{52, 20}, {56, 32}},
        {{48, 20}, {52, 32}},
        {{40, 20}, {44, 32}},
        {{44, 16}, {48, 20}},
        {{48, 16}, {52, 20}}}
    },
    armRight{{0.5f, 0.5f, 0.f}, {4._px, 12._px, 4._px}, {64, 32}, std::array<TextureCoords<int>, 6>{
        TextureCoords<int>
        {{48, 20}, {44, 32}},
        {{56, 20}, {52, 32}},
        {{40, 20}, {44, 32}},
        {{48, 20}, {52, 32}},
        {{44, 16}, {48, 20}},
        {{48, 16}, {52, 20}}}
    },
    leg{{0.f, 0.5f, 0.f}, {4._px, 12._px, 4._px}, {64, 32}, std::array<TextureCoords<int>, 6>{
        TextureCoords<int>
        {{4, 20}, {8, 32}},
        {{12, 20}, {16, 32}},
        {{8, 20}, {12, 32}},
        {{0, 20}, {4, 32}},
        {{4, 16}, {8, 20}},
        {{8, 16}, {12, 20}}}
    }
{}

void HumanModel::draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const {
    glm::mat4 baseTransform{1.f};
    baseTransform = glm::translate(baseTransform, position);

    glm::mat4 headTransform = baseTransform;
    headTransform = glm::rotate(baseTransform, state.yaw, glm::vec3{0.f, 1.f, 0.f});
    headTransform = glm::translate(headTransform, {0._px, 24._px, 0._px});
    headTransform = glm::rotate(headTransform, state.pitch, glm::vec3{1.f, 0.f, 0.f});
    head.draw(shader, headTransform);


    baseTransform = glm::rotate(baseTransform, state.bodyYaw, glm::vec3{0.f, 1.f, 0.f});

    glm::mat4 leg0Transform = baseTransform;
    leg0Transform = glm::translate(leg0Transform, {2._px, 12._px, 0._px});
    leg0Transform = glm::rotate(leg0Transform, std::sin(state.limbRotation), glm::vec3{1.f, 0.f, 0.f});
    leg.draw(shader, leg0Transform);
    glm::mat4 leg1Transform = baseTransform; 
    leg1Transform = glm::translate(leg1Transform, {-2._px, 12._px, 0._px});
    leg1Transform = glm::rotate(leg1Transform, std::sin(-state.limbRotation), glm::vec3{1.f, 0.f, 0.f});
    leg.draw(shader, leg1Transform);

    glm::mat4 torsoTransform = baseTransform;
    torsoTransform = glm::translate(torsoTransform, {0._px, 12._px, 0._px});
    torso.draw(shader, torsoTransform);

    glm::mat4 arm0Transform = baseTransform;
    arm0Transform = glm::translate(arm0Transform, {4._px, 24._px, 0._px});
    arm0Transform = glm::rotate(arm0Transform, std::sin(-state.limbRotation), glm::vec3{1.f, 0.f, 0.f});
    armLeft.draw(shader, arm0Transform);

    glm::mat4 arm1Transform = baseTransform;
    arm1Transform = glm::translate(arm1Transform, {-4._px, 24._px, 0._px});
    arm1Transform = glm::rotate(arm1Transform, std::sin(state.limbRotation), glm::vec3{1.f, 0.f, 0.f});
    armRight.draw(shader, arm1Transform);

}
