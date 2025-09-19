#include <raylib.h>
#include "util.h"
#include "block.h"
#include "mesh.h"

void meshVertex(Mesh* mesh, int index, Vector3 position, float texCoordX, float texCoordY, Vector3 normal) {
    int coordIndex = index * 3;
    mesh->vertices[coordIndex] = position.x;
    mesh->vertices[coordIndex + 1] = position.y;
    mesh->vertices[coordIndex + 2] = position.z;

    mesh->normals[coordIndex] = normal.x;
    mesh->normals[coordIndex + 1] = normal.y;
    mesh->normals[coordIndex + 2] = normal.z;

    int colorIndex = index * 4;
    mesh->colors[colorIndex] = 128;
    mesh->colors[colorIndex + 1] = 128;

    int texIndex = index * 2;
    mesh->texcoords[texIndex] = texCoordX;
    mesh->texcoords[texIndex + 1] = texCoordY;
}

void meshFace(Mesh* mesh, int index, Vector3 a, Vector3 b, Vector3 c, Vector3 d, int textureX, int textureY, Vector3 normal) {
    float blockFaceSize = 16.f / 256.f;
    float texCoordX = textureX * blockFaceSize;
    float texCoordY = textureY * blockFaceSize;

    int vertIndex = index * 4;

    meshVertex(mesh, vertIndex + 0, a, texCoordX, texCoordY + blockFaceSize, normal);
    meshVertex(mesh, vertIndex + 1, b, texCoordX + blockFaceSize, texCoordY + blockFaceSize, normal);
    meshVertex(mesh, vertIndex + 2, c, texCoordX + blockFaceSize, texCoordY, normal);
    meshVertex(mesh, vertIndex + 3, d, texCoordX, texCoordY, normal);

    int indIndex = index * 6;
    mesh->indices[indIndex + 0] = vertIndex + 0;
    mesh->indices[indIndex + 1] = vertIndex + 1;
    mesh->indices[indIndex + 2] = vertIndex + 2;
    mesh->indices[indIndex + 3] = vertIndex + 2;
    mesh->indices[indIndex + 4] = vertIndex + 3;
    mesh->indices[indIndex + 5] = vertIndex + 0;
}

void meshFaceSmart(Mesh* mesh, int index, int x, int y, int z, Direction side, int textureX, int textureY) {
    // x = 0 left / 1 right, y = 0 bottom / 1 top, z = 0 back / 1 front
    const Vector3 lbb = {x + 0, y + 0, z + 0};
    const Vector3 lbf = {x + 0, y + 0, z + 1};
    const Vector3 ltb = {x + 0, y + 1, z + 0};
    const Vector3 ltf = {x + 0, y + 1, z + 1};
    const Vector3 rbb = {x + 1, y + 0, z + 0};
    const Vector3 rbf = {x + 1, y + 0, z + 1};
    const Vector3 rtb = {x + 1, y + 1, z + 0};
    const Vector3 rtf = {x + 1, y + 1, z + 1};

    const Vector3 vertexOffsets[DIRECTION_COUNT][4] = {
        [DIRECTION_UP]      = {ltf, rtf, rtb, ltb},
        [DIRECTION_SOUTH]   = {lbf, rbf, rtf, ltf},
        [DIRECTION_EAST]    = {rbf, rbb, rtb, rtf},
        [DIRECTION_NORTH]   = {rbb, lbb, ltb, rtb},
        [DIRECTION_WEST]    = {lbb, lbf, ltf, ltb},
        [DIRECTION_DOWN]    = {lbb, rbb, rbf, lbf},
    };

    const Vector3* offsets = vertexOffsets[side];
    Vector3 normal = pointToVector3(directionToPoint(side));
    meshFace(mesh, index, offsets[0], offsets[1], offsets[2], offsets[3], textureX, textureY, normal);
}

void meshAddCube(Mesh* mesh, int index, int x, int y, int z, Block block) {
    const Point* sides = blocks[block].model.sides;

    int meshIndex = index;
    for (int dir = 0; dir < DIRECTION_COUNT; ++dir) {
        meshFaceSmart(mesh, meshIndex + 0, x, y, z, dir, sides[dir].x, sides[dir].y);
        meshIndex++;
    }
}

Mesh cubeMesh(Block block) {
    int faces = 6;

    Mesh mesh = {0};
    mesh.triangleCount = faces * 2;
    mesh.vertexCount = faces * 4;
    mesh.vertices = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.texcoords = (float*)MemAlloc(mesh.vertexCount * 2 * sizeof(float));
    mesh.normals = (float*)MemAlloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.colors = (unsigned char*)MemAlloc(mesh.vertexCount * 4 * sizeof(unsigned char));
    mesh.indices = (unsigned short*)MemAlloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    meshAddCube(&mesh, 0, 0, 0, 0, block);

    UploadMesh(&mesh, false);

    return mesh;
}

