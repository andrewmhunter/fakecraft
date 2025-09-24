#ifndef MESH_H
#define MESH_H

#include <raylib.h>
#include "block.h"
#include "mem.h"

typedef LIST_T(Mesh) MeshList;

void meshVertex(Mesh* mesh, int index, Vector3 position, float texCoordX, float texCoordY, Vector3 normal);
void meshFace(Mesh* mesh, int index, Vector3 a, Vector3 b, Vector3 c, Vector3 d, int textureX, int textureY, Vector3 normal);
void meshFaceSmart(Mesh* mesh, int index, int x, int y, int z, Direction side, int textureX, int textureY);
void meshAddCube(Mesh* mesh, int index, int x, int y, int z, Block block);
Mesh cubeMesh(Block block);

void meshListClear(MeshList* list);

#endif

