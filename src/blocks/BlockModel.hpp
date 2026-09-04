#ifndef FAKECRAFT_BLOCK_MODEL_HPP
#define FAKECRAFT_BLOCK_MODEL_HPP

#include "graphics/graphics.hpp"
#include "level/block.hpp"
#include "level/collision.hpp"
#include "util/direction.hpp"
#include "util/types.hpp"
#include <array>
#include <glm/ext/vector_int2.hpp>

using BlockState = u8;

struct BlockInstance {
    Block id;
    BlockState state;

    explicit BlockInstance(Block id, BlockState state);
    BlockInstance(Block id);
};

enum class Transparency {
    solid,
    transparent,
    translucent,
};

class Chunk;

class BlockInfo {
protected:
    bool shouldDrawSide(const Chunk& chunk, BlockInstance blockInstance, glm::ivec3 position, Direction side) const;
    explicit BlockInfo();

public:
    virtual ~BlockInfo();

    virtual BlockState getMaxBlockState() const;
    virtual bool isBlockStateValid(BlockState blockState) const;

    virtual BoundingBox getBoundingBox(BlockState blockState) const;
    virtual Passability getPassability(BlockState blockState) const;
    virtual Transparency getTransparency(BlockState blockState) const;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const;

    virtual void appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const = 0;
};

class AirBlockModel final : public BlockInfo {
public:
    virtual Transparency getTransparency(BlockState blockState) const override;
    virtual Passability getPassability(BlockState blockState) const override;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
    virtual void appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
};


struct TexturePositions {
    std::array<glm::ivec2, directionCount> values;

    TexturePositions(glm::ivec2 sides);
    TexturePositions(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side);
    TexturePositions(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 north, glm::ivec2 south, glm::ivec2 east, glm::ivec2 west);
    TexturePositions(std::array<glm::ivec2, directionCount> texturePositions);
};

class FullBlockModel final : public BlockInfo {
protected:
    const std::array<glm::ivec2, directionCount> texturePositions;
    
public:
    const Transparency transparency;
    const Passability passability;

    explicit FullBlockModel(TexturePositions texturePositions, Transparency transparency = Transparency::solid, Passability passability = Passability::impassable);

    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
    void appendGeometrySide(ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::vec3 offset, BlockInstance blockInstance, Direction side) const;
    virtual void appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
    virtual Transparency getTransparency(BlockState blockState) const override;
    virtual Passability getPassability(BlockState blockState) const override;
};

class PlantBlockModel final : public BlockInfo {
private:
    const glm::ivec2 texturePosition;
    const glm::vec3 boundingSize;
    const color::Color tint;

public:
    explicit PlantBlockModel(glm::ivec2 texturePosition, glm::vec3 boundingSize, color::Color tint = color::white);

    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;

    virtual BoundingBox getBoundingBox(BlockState blockState) const override;
    virtual Transparency getTransparency(BlockState blockState) const override;
    virtual Passability getPassability(BlockState blockState) const override;

    virtual void appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
};

class CactusBlockModel final : public BlockInfo {
private:
    glm::ivec2 topTexture;
    glm::ivec2 bottomTexture;
    glm::ivec2 sideTexture;

public:
    explicit CactusBlockModel(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side);

    virtual void appendGeometry(const Chunk& chunk, ChunkMesh& opaqueMesh, ChunkMesh& translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
    virtual Transparency getTransparency(BlockState blockState) const override;
};


class GrassBlockModel final : public BlockInfo {
private:
    glm::ivec2 topTexture;
    glm::ivec2 bottomTexture;
    glm::ivec2 sideTexture;
    glm::ivec2 sideOverlayTexture;
    color::Color tint;

public:
    explicit GrassBlockModel(glm::ivec2 top, glm::ivec2 bottom, glm::ivec2 side, glm::ivec2 sideOverlay, color::Color tint);

    virtual void appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
};

class SlabBlockModel final : public BlockInfo {
private:
    const std::array<glm::ivec2, directionCount> texturePositions;

public:
    explicit SlabBlockModel(TexturePositions texturePositions);
    virtual void appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
    virtual BoundingBox getBoundingBox(BlockState blockState) const override;
};

class TorchBlockModel final : public BlockInfo {
private:
    glm::ivec2 texturePosition;

public:
    explicit TorchBlockModel(glm::ivec2 texturePosition);

    virtual void appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
    virtual Passability getPassability(BlockState blockState) const override;
    virtual BoundingBox getBoundingBox(BlockState blockState) const override;
};

class LeavesBlockModel final : public BlockInfo {
private:
    glm::ivec2 texturePosition;
    color::Color tint;

public:
    explicit LeavesBlockModel(glm::ivec2 texturePosition, color::Color tint);

    virtual void appendGeometry(const Chunk &chunk, ChunkMesh &opaqueMesh, ChunkMesh &translucentMesh, glm::ivec3 position, BlockInstance blockInstance) const override;
    virtual bool masksSide(BlockInstance blockInstance, Direction side, BlockInstance otherBlock) const override;
};

void generateBlockModels();
const BlockInfo& getBlockModel(Block block);

#endif
