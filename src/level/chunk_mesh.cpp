#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>
#include "block.hpp"
#include "blocks/BlockModel.hpp"
#include "chunk.hpp"


std::chrono::duration<double> averageDuration;
int durationCount = 0;

void Chunk::generateMesh() {
    ChunkMesh opaqueMesh{};
    ChunkMesh translucentMesh{};

    auto start = std::chrono::high_resolution_clock::now();

    for (int y = 0; y < CHUNK_HEIGHT; ++y) {
        for (int x = 0; x < CHUNK_WIDTH; ++x) {
            for (int z = 0; z < CHUNK_WIDTH; ++z) {
                glm::ivec3 point = {x, y, z};
                Block block = blocks[x][y][z];

                if (block == Block::air) {
                    continue;
                }

                const BlockInfo& model = getBlockModel(block);
                model.appendGeometry(*this, opaqueMesh, translucentMesh, point, block);
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;
    averageDuration += duration;
    durationCount += 1;
    
    //Logger::info(std::format("Time: {}, Average: {}", duration, averageDuration / durationCount));

    cpuMesh = std::move(opaqueMesh);
    cpuTranslucentMesh = std::move(translucentMesh);

    dirty = false;
}

void Chunk::uploadMesh() {
    mesh = cpuMesh->upload();
    cpuMesh = std::nullopt;
    translucentMesh = cpuTranslucentMesh->upload();
    cpuTranslucentMesh = std::nullopt;
}
