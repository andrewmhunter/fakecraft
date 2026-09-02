#ifndef MESH_HPP
#define MESH_HPP

#include "level/block.hpp"
#include "graphics/graphics.hpp"
#include "util/direction.hpp"


void meshFace(ChunkMesh& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
    glm::ivec2 textureCoords, glm::vec3 normal, glm::vec4 color);

void meshFaceSmart(ChunkMesh& mesh, glm::ivec3 position, Direction side, glm::ivec2 textureCoords, color::Color color = color::white);

void meshFaceSmart(
    ChunkMesh& mesh,
    glm::vec3 position,
    glm::vec3 size,
    Direction side,
    glm::ivec2 textureCoordsBlock,
    glm::ivec2 textureCoordsPixel,
    glm::ivec2 textureCoordsPixelSize
);

void meshAddCube(ChunkMesh& mesh, glm::ivec3 position, Block block);
GPUMesh blockMesh(Block block);

void meshCross(ChunkMesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords, color::Color color = color::white);
void meshCactus(ChunkMesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords);

#endif
