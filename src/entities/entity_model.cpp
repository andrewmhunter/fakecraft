#include "entity_model.hpp"
#include "util/direction.hpp"
#include "graphics/graphics.hpp"
#include <cstddef>
#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/trigonometric.hpp>
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
)
    : mesh{generateMesh(origin, size, textureSize, texCoords).upload()}
{}

EntityModelPart::EntityModelPart(glm::vec3 origin, glm::ivec3 sizePixels, glm::ivec2 textureSize, glm::ivec2 netTexcoord)
    : EntityModelPart(origin, glm::vec3{sizePixels} / 16.f, textureSize, std::array<TextureCoords<int>, 6> {
        TextureCoords<int>
        {netTexcoord + glm::ivec2{sizePixels.z, sizePixels.z}, netTexcoord + glm::ivec2{sizePixels.z + sizePixels.x, sizePixels.z + sizePixels.y}},
        {netTexcoord + glm::ivec2{sizePixels.z * 2 + sizePixels.x, sizePixels.z}, netTexcoord + glm::ivec2{sizePixels.z * 2 + sizePixels.x * 2, sizePixels.z + sizePixels.y}},
        {netTexcoord + glm::ivec2{sizePixels.z + sizePixels.x, sizePixels.z}, netTexcoord + glm::ivec2{sizePixels.z * 2 + sizePixels.x, sizePixels.z + sizePixels.y}},
        {netTexcoord + glm::ivec2{0, sizePixels.z}, netTexcoord + glm::ivec2{sizePixels.z, sizePixels.z + sizePixels.y}},
        {netTexcoord + glm::ivec2{sizePixels.z, 0}, netTexcoord + glm::ivec2{sizePixels.z + sizePixels.x, sizePixels.z}},
        {netTexcoord + glm::ivec2{sizePixels.z + sizePixels.x, 0}, netTexcoord + glm::ivec2{sizePixels.z + sizePixels.x * 2, sizePixels.z}},
    })
{}

void EntityModelPart::draw(ShaderProgram& shader, glm::mat4 transform) const {
    shader.setModel(transform);
    mesh.draw();
}

HumanModelState::HumanModelState(float yaw, float bodyYaw, float pitch, float limbRotation)
    : yaw{yaw}, bodyYaw{bodyYaw}, pitch{pitch}, limbRotation{limbRotation}
{}

HumanModel::HumanModel()
    : head{{0.f, -0.5f, 0.f}, glm::ivec3{8}, {64, 32}, {0, 0}},
    torso{{0.f, -0.5f, 0.f}, {8, 12, 4}, {64, 32}, {16, 16}},
    armLeft{{-0.5f, 0.5f, 0.f}, {4, 12, 4}, {64, 32}, {40, 16}},
    armRight{{0.5f, 0.5f, 0.f}, {4, 12, 4}, {64, 32}, {40, 16}},
    leg{{0.f, 0.5f, 0.f}, {4, 12, 4}, {64, 32}, {0, 16}}
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


PigModel::PigModel()
    : head{{0.f, -0.5f, -0.5f}, glm::ivec3{8}, {64, 32}, {0, 0}},
    torso{{0.f, -0.5f, 0.5f}, {10, 16, 8}, {64, 32}, {28, 8}},
    leg{{0.f, 0.5f, 0.f}, {4, 6, 4}, {64, 32}, {0, 16}}
{}
    
void PigModel::draw(ShaderProgram& shader, glm::vec3 position, const HumanModelState& state) const {
    glm::mat4 baseTransform{1.f};
    baseTransform = glm::translate(baseTransform, position);

    baseTransform = glm::rotate(baseTransform, state.bodyYaw, glm::vec3{0.f, 1.f, 0.f});

    glm::mat4 headTransform = baseTransform;
    headTransform = glm::translate(headTransform, {0._px, 10._px, 6._px});
    headTransform = glm::rotate(headTransform, state.yaw - state.bodyYaw, glm::vec3{0.f, 1.f, 0.f});
    headTransform = glm::rotate(headTransform, state.pitch, glm::vec3{1.f, 0.f, 0.f});
    head.draw(shader, headTransform);


    glm::mat4 leg0Rotate = glm::rotate(glm::mat4{1.f}, std::sin(state.limbRotation), glm::vec3{1.f, 0.f, 0.f});
    glm::mat4 leg1Rotate = glm::rotate(glm::mat4{1.f}, std::sin(-state.limbRotation), glm::vec3{1.f, 0.f, 0.f});

    glm::mat4 legRF = glm::translate(baseTransform, {3._px, 6._px, 6._px});
    leg.draw(shader, legRF * leg0Rotate);
    glm::mat4 legRB = glm::translate(baseTransform, {3._px, 6._px, -6._px});
    leg.draw(shader, legRB * leg1Rotate);
    glm::mat4 legLF = glm::translate(baseTransform, {-3._px, 6._px, 6._px});
    leg.draw(shader, legLF * leg1Rotate);
    glm::mat4 legLB = glm::translate(baseTransform, {-3._px, 6._px, -6._px});
    leg.draw(shader, legLB * leg0Rotate);

    glm::mat4 torsoTransform = baseTransform;
    torsoTransform = glm::translate(torsoTransform, {0._px, 6._px, -8._px});
    torsoTransform = glm::rotate(torsoTransform, glm::radians(90.f), glm::vec3{1.f, 0.f, 0.f});

    torso.draw(shader, torsoTransform);
}
