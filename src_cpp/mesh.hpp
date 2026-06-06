#ifndef MESH_HPP
#define MESH_HPP

#include "block.hpp"
#include "graphics.hpp"


void meshFace(Mesh& mesh, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d,
        int textureX, int textureY, glm::vec3 normal, glm::vec4 color);

void meshFaceSmart(Mesh& mesh, int x, int y, int z, Direction side, int textureX, int textureY);
void meshAddCube(Mesh& mesh, int x, int y, int z, Block block);
GPUMesh blockMesh(Block block);

void meshCross(Mesh& mesh, int x, int y, int z, int textureX, int textureY);

#endif

