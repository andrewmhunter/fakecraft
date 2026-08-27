#include "graphics/graphics.hpp"
#include "level/block.hpp"
#include "graphics/mesh.hpp"
#include "util/direction.hpp"
#include "util/util.hpp"
#include <glm/fwd.hpp>

void meshFace(
        Mesh& mesh,
        glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
        glm::ivec2 textureCoords,
        glm::vec3 normal,
        glm::vec4 color
) {
    float blockFaceSize = 16.f / 256.f;
    glm::vec2 texCoord0 = glm::vec2{textureCoords} * blockFaceSize;
    glm::vec2 texCoord1 = texCoord0 + glm::vec2{blockFaceSize};

    mesh.pushFace(a, b, c, d, texCoord0, texCoord1, color, normal);
}

void meshFaceSmart(Mesh& mesh,
    glm::vec3 position,
    glm::vec3 size,
    Direction side,
    glm::ivec2 textureCoordsBlock,
    glm::ivec2 textureCoordsPixel,
    glm::ivec2 textureCoordsPixelSize
) {
    float x = position.x;
    float y = position.y;
    float z = position.z;

    float ox = size.x / 2.f;
    float oy = size.y / 2.f;
    float oz = size.z / 2.f;

    // x = 0 left / 1 right, y = 0 bottom / 1 top, z = 0 back / 1 front
    const glm::vec3 lbb = {x - ox, y - oy, z - oz};
    const glm::vec3 lbf = {x - ox, y - oy, z + oz};
    const glm::vec3 ltb = {x - ox, y + oy, z - oz};
    const glm::vec3 ltf = {x - ox, y + oy, z + oz};
    const glm::vec3 rbb = {x + ox, y - oy, z - oz};
    const glm::vec3 rbf = {x + ox, y - oy, z + oz};
    const glm::vec3 rtb = {x + ox, y + oy, z - oz};
    const glm::vec3 rtf = {x + ox, y + oy, z + oz};

    const glm::vec3 vertexOffsets[directionCount][4] = {
        {ltf, lbf, rbf, rtf},
        {rtf, rbf, rbb, rtb},
        {rtb, rbb, lbb, ltb},
        {ltb, lbb, lbf, ltf},
        {ltb, ltf, rtf, rtb},
        {lbf, lbb, rbb, rbf},
    };

    const glm::vec3* offsets = vertexOffsets[side];
    glm::vec3 normal{directionToPoint(side)};

    float blockFaceSize = 16.f / 256.f;
    glm::vec2 texCoord0 = glm::vec2{textureCoordsBlock} * blockFaceSize + glm::vec2{textureCoordsPixel} / 256.f;
    glm::vec2 texCoord1 = texCoord0 + glm::vec2{textureCoordsPixelSize} / 256.f;

    mesh.pushFace(offsets[0], offsets[1], offsets[2], offsets[3], texCoord0, texCoord1, color::white, normal);
}

void meshFaceSmart(Mesh& mesh, glm::ivec3 position, Direction side, glm::ivec2 textureCoords, color::Color color) {
    int x = position.x;
    int y = position.y;
    int z = position.z;

    // x = 0 left / 1 right, y = 0 bottom / 1 top, z = 0 back / 1 front
    const glm::vec3 lbb = {x + 0.f, y + 0.f, z + 0.f};
    const glm::vec3 lbf = {x + 0.f, y + 0.f, z + 1.f};
    const glm::vec3 ltb = {x + 0.f, y + 1.f, z + 0.f};
    const glm::vec3 ltf = {x + 0.f, y + 1.f, z + 1.f};
    const glm::vec3 rbb = {x + 1.f, y + 0.f, z + 0.f};
    const glm::vec3 rbf = {x + 1.f, y + 0.f, z + 1.f};
    const glm::vec3 rtb = {x + 1.f, y + 1.f, z + 0.f};
    const glm::vec3 rtf = {x + 1.f, y + 1.f, z + 1.f};

    const glm::vec3 vertexOffsets[directionCount][4] = {
        {ltf, lbf, rbf, rtf},
        {rtf, rbf, rbb, rtb},
        {rtb, rbb, lbb, ltb},
        {ltb, lbb, lbf, ltf},
        {ltb, ltf, rtf, rtb},
        {lbf, lbb, rbb, rbf},
    };

    const glm::vec3* offsets = vertexOffsets[side];
    glm::vec3 normal{directionToPoint(side)};

    //lm::vec4 color = {128.f, 128.f, 0.f, 255.f};
    //color /= 255;

    meshFace(mesh, offsets[0], offsets[1], offsets[2], offsets[3], textureCoords, normal, color);
}

void meshAddCube(Mesh& mesh, glm::ivec3 position, Block block) {
    const glm::ivec3* sides = getBlockProperties(block).model.sides;

    for (int dir = 0; dir < directionCount; ++dir) {
        meshFaceSmart(mesh, position, static_cast<Direction>(dir), sides[dir]);
    }
}

GPUMesh blockMesh(Block block) {
    Mesh mesh{};
    meshAddCube(mesh, glm::ivec3{0}, block);

    return mesh.upload();
}

void meshCross(Mesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords, color::Color color) {
    int x = position.x;
    int y = position.y;
    int z = position.z;

    glm::vec3 lbb = {x + 0.f, y + 0.f, z + 0.f};
    glm::vec3 lbf = {x + 0.f, y + 0.f, z + 1.f};
    glm::vec3 ltb = {x + 0.f, y + 1.f, z + 0.f};
    glm::vec3 ltf = {x + 0.f, y + 1.f, z + 1.f};
    glm::vec3 rbb = {x + 1.f, y + 0.f, z + 0.f};
    glm::vec3 rbf = {x + 1.f, y + 0.f, z + 1.f};
    glm::vec3 rtb = {x + 1.f, y + 1.f, z + 0.f};
    glm::vec3 rtf = {x + 1.f, y + 1.f, z + 1.f};

    glm::vec3 normal = {0.f, 1.f, 0.f};

    meshFace(mesh, ltb, lbb, rbf, rtf, textureCoords, normal, color);
    meshFace(mesh, rtf, rbf, lbb, ltb, textureCoords, normal, color);
    meshFace(mesh, ltf, lbf, rbb, rtb, textureCoords, normal, color);
    meshFace(mesh, rtb, rbb, lbf, ltf, textureCoords, normal, color);
}

static void meshFaceOffset(
        Mesh& mesh,
        glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
        glm::ivec2 textureCoords,
        glm::vec3 normal,
        glm::vec4 color,
        glm::vec3 offset
) {
    meshFace(mesh, a + offset, b + offset, c + offset, d + offset, textureCoords, normal, color);
}

void meshCactus(Mesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords) {
    int x = position.x;
    int y = position.y;
    int z = position.z;

    glm::vec3 lbb = {x + 0.f, y + 0.f, z + 0.f};
    glm::vec3 lbf = {x + 0.f, y + 0.f, z + 1.f};
    glm::vec3 ltb = {x + 0.f, y + 1.f, z + 0.f};
    glm::vec3 ltf = {x + 0.f, y + 1.f, z + 1.f};
    glm::vec3 rbb = {x + 1.f, y + 0.f, z + 0.f};
    glm::vec3 rbf = {x + 1.f, y + 0.f, z + 1.f};
    glm::vec3 rtb = {x + 1.f, y + 1.f, z + 0.f};
    glm::vec3 rtf = {x + 1.f, y + 1.f, z + 1.f};

    glm::vec4 color = color::white;

    glm::vec3 southOffset = glm::vec3{directionToPoint(Direction::north)} * static_cast<float>(1._px);
    glm::vec3 eastOffset = glm::vec3{directionToPoint(Direction::west)} * static_cast<float>(1._px);
    glm::vec3 northOffset = glm::vec3{directionToPoint(Direction::south)} * static_cast<float>(1._px);
    glm::vec3 westOffset = glm::vec3{directionToPoint(Direction::east)} * static_cast<float>(1._px);

    meshFaceOffset(mesh, ltf, lbf, rbf, rtf, textureCoords, directionToPoint(Direction::south), color, southOffset);
    meshFaceOffset(mesh, rtf, rbf, lbf, ltf, textureCoords, directionToPoint(Direction::south), color, southOffset);

    meshFaceOffset(mesh, rtf, rbf, rbb, rtb, textureCoords, directionToPoint(Direction::east), color, eastOffset);
    meshFaceOffset(mesh, rtb, rbb, rbf, rtf, textureCoords, directionToPoint(Direction::east), color, eastOffset);

    meshFaceOffset(mesh, rtb, rbb, lbb, ltb, textureCoords, directionToPoint(Direction::north), color, northOffset);
    meshFaceOffset(mesh, ltb, lbb, rbb, rtb, textureCoords, directionToPoint(Direction::north), color, northOffset);

    meshFaceOffset(mesh, ltb, lbb, lbf, ltf, textureCoords, directionToPoint(Direction::west), color, westOffset);
    meshFaceOffset(mesh, ltf, lbf, lbb, ltb, textureCoords, directionToPoint(Direction::west), color, westOffset);
}
