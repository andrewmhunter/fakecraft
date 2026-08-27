#ifndef MESH_HPP
#define MESH_HPP

#include "entities/entity_model.hpp"
#include "level/block.hpp"
#include "graphics/graphics.hpp"
#include "util/direction.hpp"


void meshFace(Mesh& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
    glm::ivec2 textureCoords, glm::vec3 normal, glm::vec4 color);

void meshFaceSmart(Mesh& mesh, glm::ivec3 position, Direction side, glm::ivec2 textureCoords, color::Color color = color::white);

void meshFaceSmart(
    Mesh& mesh,
    glm::vec3 position,
    glm::vec3 size,
    Direction side,
    glm::ivec2 textureCoordsBlock,
    glm::ivec2 textureCoordsPixel,
    glm::ivec2 textureCoordsPixelSize
);

void meshAddCube(Mesh& mesh, glm::ivec3 position, Block block);
GPUMesh blockMesh(Block block);

void meshCross(Mesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords, color::Color color = color::white);
void meshCactus(Mesh& mesh, glm::ivec3 position, glm::ivec2 textureCoords);

#endif
