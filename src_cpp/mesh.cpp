#include "block.hpp"
#include "mesh.hpp"

void meshFace(
        Mesh& mesh,
        glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
        int textureX, int textureY,
        glm::vec3 normal,
        glm::vec4 color
) {
    float blockFaceSize = 16.f / 256.f;
    glm::vec2 texCoord0{textureX * blockFaceSize, textureY * blockFaceSize};
    glm::vec2 texCoord1 = texCoord0 + glm::vec2{blockFaceSize};

    mesh.pushFace(a, b, c, d, texCoord0, texCoord1, color, normal);
}

void meshFaceSmart(
        Mesh& mesh,
        int x, int y, int z,
        Direction side,
        int textureX, int textureY
) {
    // x = 0 left / 1 right, y = 0 bottom / 1 top, z = 0 back / 1 front
    const glm::vec3 lbb = {x + 0.f, y + 0.f, z + 0.f};
    const glm::vec3 lbf = {x + 0.f, y + 0.f, z + 1.f};
    const glm::vec3 ltb = {x + 0.f, y + 1.f, z + 0.f};
    const glm::vec3 ltf = {x + 0.f, y + 1.f, z + 1.f};
    const glm::vec3 rbb = {x + 1.f, y + 0.f, z + 0.f};
    const glm::vec3 rbf = {x + 1.f, y + 0.f, z + 1.f};
    const glm::vec3 rtb = {x + 1.f, y + 1.f, z + 0.f};
    const glm::vec3 rtf = {x + 1.f, y + 1.f, z + 1.f};

    const glm::vec3 vertexOffsets[DIRECTION_COUNT][4] = {
        {ltf, lbf, rbf, rtf},
        {rtf, rbf, rbb, rtb},
        {rtb, rbb, lbb, ltb},
        {ltb, lbb, lbf, ltf},
        {ltb, ltf, rtf, rtb},
        {lbf, lbb, rbb, rbf},
    };

    const glm::vec3* offsets = vertexOffsets[side];
    glm::vec3 normal{directionToPoint(side)};

    glm::vec4 color = {128, 128, 0, 255};
    color /= 255;

    meshFace(mesh, offsets[0], offsets[1], offsets[2], offsets[3], textureX, textureY, normal, color);
}

void meshAddCube(Mesh& mesh, int x, int y, int z, Block block) {
    const glm::ivec3* sides = blocks[block].model.sides;

    for (int dir = 0; dir < DIRECTION_COUNT; ++dir) {
        meshFaceSmart(mesh, x, y, z, static_cast<Direction>(dir), sides[dir].x, sides[dir].y);
    }
}

GPUMesh blockMesh(Block block) {
    Mesh mesh{};
    meshAddCube(mesh, 0, 0, 0, block);

    return mesh.upload();
}

void meshCross(Mesh& mesh, int x, int y, int z, int textureX, int textureY) {
    glm::vec3 lbb = {x + 0.f, y + 0.f, z + 0.f};
    glm::vec3 lbf = {x + 0.f, y + 0.f, z + 1.f};
    glm::vec3 ltb = {x + 0.f, y + 1.f, z + 0.f};
    glm::vec3 ltf = {x + 0.f, y + 1.f, z + 1.f};
    glm::vec3 rbb = {x + 1.f, y + 0.f, z + 0.f};
    glm::vec3 rbf = {x + 1.f, y + 0.f, z + 1.f};
    glm::vec3 rtb = {x + 1.f, y + 1.f, z + 0.f};
    glm::vec3 rtf = {x + 1.f, y + 1.f, z + 1.f};

    //Vector3 normal = pointToVector3(directionToPoint(side));
    glm::vec3 normal0 = {0.f, 1.f, 0.f};
    glm::vec3 normal1 = {1.f, 0.f, 0.f};
    glm::vec4 color = {128, 128, 0, 255};
    color /= 255.f;

    meshFace(mesh, ltb, lbb, rbf, rtf, textureX, textureY, normal0, color);
    meshFace(mesh, rtf, rbf, lbb, ltb, textureX, textureY, normal1, color);
    meshFace(mesh, ltf, lbf, rbb, rtb, textureX, textureY, normal1, color);
    meshFace(mesh, rtb, rbb, lbf, ltf, textureX, textureY, normal0, color);
}

