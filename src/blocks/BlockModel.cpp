#include "blocks/BlockModel.hpp"
#include "graphics/graphics.hpp"
#include "graphics/mesh.hpp"
#include "level/block.hpp"
#include "level/collision.hpp"
#include "level/chunk.hpp"
#include "util/direction.hpp"
#include <array>
#include <concepts>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>
#include <memory>
#include <utility>

static std::array<std::unique_ptr<BlockInfo>, blockCount> blockRegistry;

FullBlockModel invalidBlockModel{glm::ivec2{15, 1}};

template<std::derived_from<BlockInfo> T, typename... Args>
static void newBlock(Block blockId, Args&&... args) {
    blockRegistry.at(static_cast<int>(blockId)) = std::make_unique<T>(std::forward<Args>(args)...);
}

void generateBlockModels() {
    color::Color grassColor = glm::vec4{0.749f, 1.253f, 0.418f, 1.0f};

    newBlock<FullBlockModel>(Block::barrier, glm::ivec2{0, 0});
    newBlock<AirBlockModel>(Block::air);

    newBlock<FullBlockModel>(Block::stone, glm::ivec2{1, 0});
    newBlock<FullBlockModel>(Block::dirt, glm::ivec2{2, 0});
    newBlock<FullBlockModel>(Block::planks, glm::ivec2{4, 0});
    newBlock<FullBlockModel>(Block::cobblestone, glm::ivec2{0, 1});
    newBlock<FullBlockModel>(Block::bedrock, glm::ivec2{1, 1});
    newBlock<FullBlockModel>(Block::diamondOre, glm::ivec2{2, 3});
    newBlock<FullBlockModel>(Block::obsidian, glm::ivec2{5, 2});
    newBlock<FullBlockModel>(Block::sand, glm::ivec2{2, 1});
    newBlock<GrassBlockModel>(Block::grass, glm::ivec2{0, 0}, glm::ivec2{2, 0}, glm::ivec2{3, 0}, glm::ivec2{6, 2}, grassColor);
    newBlock<FullBlockModel>(Block::glass, glm::ivec2{1, 3}, Transparency::transparent);

    if (Config::settings->graphics.fastLeaves) {
        newBlock<FullBlockModel>(Block::leaves, glm::ivec2{5, 3});
    } else {
        newBlock<FullBlockModel>(Block::leaves, glm::ivec2{4, 3}, Transparency::transparent);
    }

    newBlock<FullBlockModel>(Block::log, TexturePositions{{5, 1}, {5, 1}, {4, 1}});
    newBlock<FullBlockModel>(Block::craftingTable, TexturePositions{{11, 2}, {4, 0}, {11, 3}, {11, 3}, {12, 3}, {12, 3}});

    newBlock<FullBlockModel>(Block::water, glm::ivec2{13, 12}, Transparency::translucent, Passability::passable);
    newBlock<FullBlockModel>(Block::snow, glm::ivec2{2, 4});
    newBlock<FullBlockModel>(Block::ice, glm::ivec2{3, 4}, Transparency::translucent);

    newBlock<CactusBlockModel>(Block::cactus, glm::ivec2{5, 4}, glm::ivec2{7, 4}, glm::ivec2{6, 4});

    newBlock<FullBlockModel>(Block::lava, glm::ivec2{13, 14}, Transparency::transparent, Passability::passable);
    newBlock<FullBlockModel>(Block::snowyGrass, TexturePositions{glm::ivec2{2, 4}, glm::ivec2{2, 0}, glm::ivec2{4, 4}});

    newBlock<PlantBlockModel>(Block::cobweb, glm::ivec2{11, 0}, glm::vec3{1.f});
    newBlock<PlantBlockModel>(Block::rose, glm::ivec2{12, 0}, glm::vec3{0.5f});
    newBlock<PlantBlockModel>(Block::dandelion, glm::ivec2{13, 0}, glm::vec3{0.5f});
    newBlock<PlantBlockModel>(Block::tallGrass, glm::ivec2{7, 2}, glm::vec3{1.f, 0.5f, 1.f}, grassColor);

    newBlock<SlabBlockModel>(Block::smoothStoneSlab, TexturePositions{{6, 0}, {6, 0}, {5, 0}});

    newBlock<TorchBlockModel>(Block::torch, glm::ivec2{0, 5});



    for (int i = 0; i < blockCount; ++i) {
        if (blockRegistry[i] == nullptr) {
            Logger::error(std::format("Missing block model for block id {}", i));
        }
    }
}

const BlockInfo& getBlockModel(Block block) {
    const BlockInfo* model = blockRegistry.at(static_cast<int>(block)).get();
    if (model == nullptr) {
        return invalidBlockModel;
    }
    return *model;
} 


BlockInstance::BlockInstance(Block id, BlockState state)
    : id{id}, state{state}
{}

BlockInstance::BlockInstance(Block id)
    : BlockInstance{id, 0}
{}


// Ignore unused parameter warnings since parameters are intentionally unused in many
// of the virtual methods because some child classes may want to use them while others don't
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


BlockInfo::BlockInfo() {}
BlockInfo::~BlockInfo() {}

BlockState BlockInfo::getMaxBlockState() const {
    return 0;
}

bool BlockInfo::isBlockStateValid(BlockState blockState) const {
    return blockState == 0;
}

BoundingBox BlockInfo::getBoundingBox(BlockState blockState) const {
    return BoundingBox{glm::vec3{0.f}, glm::vec3{1.f}};
}

Passability BlockInfo::getPassability(BlockState blockState) const {
    return Passability::impassable;
}

Transparency BlockInfo::getTransparency(BlockState blockState) const {
    return Transparency::solid;
}

bool BlockInfo::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return true;
}

bool BlockInfo::shouldDrawSide(const Chunk& chunk, BlockInstance blockInstance, glm::ivec3 position, Direction side) const {
    glm::ivec3 adjacentPosition = position + directionToIvec3(side);
    Block adjacentBlock = chunk.adjacentChunks.getBlockLocal(adjacentPosition);
    const BlockInfo& adjacentBlockModel = getBlockModel(adjacentBlock);
    return !adjacentBlockModel.masksSide(adjacentBlock, invertDirection(side), blockInstance);
}


Transparency AirBlockModel::getTransparency(BlockState blockState) const {
    return Transparency::transparent;
}

Passability AirBlockModel::getPassability(BlockState blockState) const {
    return Passability::passable;
}

bool AirBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return false;
}

void AirBlockModel::appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
}



TexturePositions::TexturePositions(glm::ivec2 sides)
    : TexturePositions{sides, sides, sides}
{}

TexturePositions::TexturePositions(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side)
    : TexturePositions{top, bottom, side, side, side, side}
{}

TexturePositions::TexturePositions(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 north, glm::ivec2 south, glm::ivec2 east, glm::ivec2 west)
    : TexturePositions{std::array{south, east, north, west, top, bottom}}
{}

TexturePositions::TexturePositions(std::array<glm::ivec2, directionCount> texturePositions)
    : values{texturePositions}
{}


FullBlockModel::FullBlockModel(TexturePositions texturePositions, Transparency transparency, Passability passability)
    : texturePositions{texturePositions.values},
    transparency{transparency},
    passability{passability}
{}

void FullBlockModel::appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    for (Direction side : directions) {
        if (shouldDrawSide(chunk, blockInstance, position, side)) {
            appendGeometrySide(opaqueMesh, translucentMesh, position, blockInstance, side);
        }
    }
}

bool FullBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    if (transparency == Transparency::solid) {
        return true;
    }
    return blockInstance.id == otherBlock.id;
}

void FullBlockModel::appendGeometrySide(ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::vec3 offset, BlockInstance blockInstance, Direction side) const {
    glm::ivec2 sideTexturePosition = texturePositions[side];
    ChunkMesh& mesh = transparency == Transparency::translucent ? translucentMesh : opaqueMesh;
    meshFaceSmart(mesh, offset, side, sideTexturePosition);
}

Passability FullBlockModel::getPassability(BlockState blockState) const {
    return passability;
}


Transparency FullBlockModel::getTransparency(BlockState blockState) const {
    return transparency;
}

PlantBlockModel::PlantBlockModel(glm::ivec2 texturePosition, glm::vec3 boundingSize, color::Color tint)
    : texturePosition{texturePosition},
    boundingSize{boundingSize},
    tint{tint}
{}

bool PlantBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return false;
}

BoundingBox PlantBlockModel::getBoundingBox(BlockState  blockState) const {
    // TODO: custom size
    return BoundingBox{glm::vec3{0.f}, glm::vec3{1.f}};
}

Transparency PlantBlockModel::getTransparency(BlockState blockState) const {
    return Transparency::transparent;
}

Passability PlantBlockModel::getPassability(BlockState blockState) const {
    return Passability::passable;
}

void PlantBlockModel::appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    meshCross(opaqueMesh, position, texturePosition, tint);
}


CactusBlockModel::CactusBlockModel(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side)
    : topTexture{top},
    bottomTexture{bottom},
    sideTexture{side}
{}

void CactusBlockModel::appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    if (shouldDrawSide(chunk, blockInstance, position, Direction::up)) {
        meshFaceSmart(opaqueMesh, position, Direction::up, topTexture);
    }

    if (shouldDrawSide(chunk, blockInstance, position, Direction::down)) {
        meshFaceSmart(opaqueMesh, position, Direction::down, bottomTexture);
    }

    meshCactus(opaqueMesh, position, sideTexture);
}

bool CactusBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return blockInstance.id == otherBlock.id && (side == Direction::up || side == Direction::down);
}

Transparency CactusBlockModel::getTransparency(BlockState blockState) const {
    return Transparency::transparent;
}


GrassBlockModel::GrassBlockModel(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side, glm::ivec2 sideOverlay, color::Color tint)
    : topTexture{top},
    bottomTexture{bottom},
    sideTexture{side},
    sideOverlayTexture{sideOverlay},
    tint{tint}
{}

void GrassBlockModel::appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    if (shouldDrawSide(chunk, blockInstance, position, Direction::up)) {
        meshFaceSmart(opaqueMesh, position, Direction::up, topTexture, tint);
    }

    for (Direction side : cardinalDirections) {
        if (shouldDrawSide(chunk, blockInstance, position, side)) {
            meshFaceSmart(opaqueMesh, position, side, sideTexture);
        }
    }

    if (shouldDrawSide(chunk, blockInstance, position, Direction::down)) {
        meshFaceSmart(opaqueMesh, position, Direction::down, bottomTexture);
    }
}


SlabBlockModel::SlabBlockModel(TexturePositions texturePositions)
    : texturePositions{texturePositions.values}
{}


void SlabBlockModel::appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    if (shouldDrawSide(chunk, blockInstance, position, Direction::down)) {
        meshFaceSmart(opaqueMesh, position, Direction::down, texturePositions[Direction::down]);
    }

    glm::vec3 centerBottomSlab = glm::vec3{position} + glm::vec3{0.5f, 0.25f, 0.5f};
    

    glm::vec3 size{1.f, 0.5f, 1.f};

    for (Direction side : cardinalDirections) {
        if (shouldDrawSide(chunk, blockInstance, position, side)) {
            meshFaceSmart(opaqueMesh, centerBottomSlab, size, side, texturePositions[side], {0, 8}, {16, 8});
        }
    }
    
    glm::vec3 center = glm::vec3{position} + glm::vec3{0.5f};
    meshFaceSmart(opaqueMesh, center, glm::vec3{1.f, 0.f, 1.f}, Direction::up, texturePositions[Direction::up], glm::ivec2{0}, glm::ivec2{16});
}

bool SlabBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return side == Direction::down;
}

BoundingBox SlabBlockModel::getBoundingBox(BlockState blockState) const {
    return BoundingBox{glm::vec3{0.f}, glm::vec3{1.f, 0.5f, 1.f}};
}


TorchBlockModel::TorchBlockModel(glm::ivec2 texturePosition)
    : texturePosition{texturePosition}
{}


void TorchBlockModel::appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const {
    glm::vec3 center = glm::vec3{position} + glm::vec3{0.5f, 5._px, 0.5f};
    glm::vec3 size{2._px, 10._px, 2._px};

    if (shouldDrawSide(chunk, blockInstance, position, Direction::down)) {
        meshFaceSmart(
            opaqueMesh,
            center,
            size,
            Direction::down,
            texturePosition,
            glm::ivec2{7, 14},
            glm::ivec2{2}
        );
    }

    for (Direction side : cardinalDirections) {
        meshFaceSmart(
            opaqueMesh,
            center,
            size,
            side,
            texturePosition,
            glm::ivec2{7, 6},
            glm::ivec2{2, 10}
        );
    }

    meshFaceSmart(
        opaqueMesh,
        center,
        size,
        Direction::up,
        texturePosition,
        glm::ivec2{7, 6},
        glm::ivec2{2, 2}
    );
}

bool TorchBlockModel::masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const {
    return false;
}

Passability TorchBlockModel::getPassability(BlockState blockState) const {
    return Passability::passable;
}

BoundingBox TorchBlockModel::getBoundingBox(BlockState blockState) const {
    return BoundingBox{glm::vec3{0.5f - 1._px, 0.f, 0.5f - 1._px}, glm::vec3{0.5f + 1._px, 10._px, 0.5f + 1._px}};
}


#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
