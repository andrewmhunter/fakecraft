#include <filesystem>
#include <format>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <utility>
#include "engine/config.hpp"
#include "engine/logger.hpp"
#include "chunk.hpp"
#include "level/collision.hpp"
#include "serialization/serialize.hpp"
#include "util/direction.hpp"
#include "world.hpp"
#include "chunk.hpp"
#include "worldgen.hpp"
#include "engine/logger.hpp"
#include "graphics/graphics.hpp"


Chunk::Chunk(World* world, glm::ivec3 coords)
    : world{world},
    coords{coords},
    adjacentChunks{coords, this}
{
    Logger::assertion(world);
}

Chunk::~Chunk() {
    unloadChunkCache();

    if (state == ChunkState::loaded) {
        unload();
    }
}

Chunk::Chunk() {}

void Chunk::generateOrLoad() {
    state = ChunkState::generating;

    if (deserialize()) {
        Logger::trace(std::format("Chunk {}, {} loaded from file", coords.x, coords.z));
        return;
    }

    generateTerrain(this);
    placeFeatures(this);
    
    Logger::trace(std::format("Chunk {}, {} generated", coords.x, coords.z));
}

void Chunk::fillChunkCache() {
    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            if (x == 0 && z == 0) {
                continue;
            }

            glm::ivec3 localChunkPosition{x, 0, z};
            Chunk* adjacent = world->getChunk(coords + localChunkPosition, ChunkState::terrainGenerated);
            adjacentChunks.setChunk(localChunkPosition, adjacent);
            if (adjacent != nullptr) {
                adjacent->adjacentChunks.setChunk(-localChunkPosition, this);
                //adjacent->dirty = true;
            }
        }
    }
}

void Chunk::unload() {
    state = ChunkState::unloaded;

    serialize();
    Logger::trace(std::format("Chunk {}, {} saved", coords.x, coords.z));
}

void Chunk::unloadChunkCache() {
    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            if (x == 0 && z == 0) {
                continue;
            }

            glm::ivec3 localChunkPosition{x, 0, z};
            Chunk* adjacent = adjacentChunks.getChunkLocal(localChunkPosition);
            adjacentChunks.setChunk(localChunkPosition, nullptr);
            if (adjacent != nullptr) {
                adjacent->adjacentChunks.setChunk(-localChunkPosition, nullptr);
            }
        }
    }
}

void Chunk::tryPlaceBlock(glm::ivec3 local, Block block) {
    if (getBlock(local) != Block::air) {
        return;
    }

    setBlock(local, block);
}

void Chunk::tryPlaceBlock(int x, int y, int z, Block block) {
    tryPlaceBlock(glm::ivec3{x, y, z}, block);
}


void Chunk::setBlockRaw(glm::ivec3 local, Block block) {
    if (!blockInChunk(local)) {
        Logger::warning("Block placed outside of chunk");
        return;
    }

    blocks[local.x][local.y][local.z] = block;
    markDirty(local);
}

void Chunk::setBlock(glm::ivec3 local, Block block) {
    setBlockRaw(local, block);

    glm::ivec3 worldPoint = localToWorld(coords, local);

    for (int i = 0; i < directionCount; ++i) {
        world->markDirty(worldPoint + directionToPoint(static_cast<Direction>(i)));
    }
}

void Chunk::tryPlaceBox(glm::ivec3 start, glm::ivec3 size, Block block) {
    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                tryPlaceBlock(glm::ivec3{x, y, z}, block);
            }
        }
    }
}

void Chunk::placeBox(glm::ivec3 start, glm::ivec3 size, Block block) {
    glm::ivec3 end = start + size;
    glm::ivec3 realStart = glm::min(start, end);
    glm::ivec3 realEnd = glm::max(start, end);

    for (int x = realStart.x; x < realEnd.x; ++x) {
        for (int y = realStart.y; y < realEnd.y; ++y) {
            for (int z = realStart.z; z < realEnd.z; ++z) {
                setBlock(glm::ivec3{x, y, z}, block);
            }
        }
    }
}

void Chunk::markDirty(glm::ivec3 local) {
    Logger::assertion(blockInChunk(local));

    dirty = true;
}

Block Chunk::getBlock(glm::ivec3 local) const {
    if (!blockInChunk(local)) {
        return Block::air;
    }
    return blocks[local.x][local.y][local.z];
}

void Chunk::drawMesh(ShaderProgram& shader, const GPUMesh& mesh) const {
    glm::mat4 transform = glm::mat4{1.f};
    transform = glm::translate(transform, glm::vec3{coords * chunkSize});

    if (state != ChunkState::loaded) {
        Logger::error(std::format("Drawing unloaded chunk with vao: {}", mesh.vertexArrayObject.object));
    }

    shader.setModel(transform);
    mesh.draw();
}

void Chunk::draw(ShaderProgram& shader) const {
    drawMesh(shader, mesh.value());
}

void Chunk::drawTranslucent(ShaderProgram& shader) const {
    drawMesh(shader, translucentMesh.value());
}

bool Chunk::verify() const {
    Logger::trace(std::format("Verifying chunk {}, {}", coords.x, coords.z));

    ITERATE_CHUNK(x, y, z) {
        Block block = blocks[x][y][z];
        if (static_cast<int>(block) >= blockCount) {
            Logger::error(std::format("Chunk verification failed. Chunk: {}, {}. Block: {}, {}, {}",
                    coords.x, coords.z, x, y, z));
            return false;
        }
    }
    return true;    
}

bool Chunk::atLeastInState(ChunkState atLeastState) const {
    return std::to_underlying(state.load()) >= std::to_underlying(atLeastState);
}

bool Chunk::adjacentCardinalsAtLeastInState(ChunkState atLeastState) const {
    for (Direction direction : cardinalDirections) {
        const Chunk* adjacent = adjacentChunks.getChunkDirection(direction);
        if (!adjacent || !adjacent->atLeastInState(atLeastState)) {
            return false;
        }
    }
    return true;
}

bool Chunk::adjacentChunksAtLeastInState(ChunkState atLeastState) const {
    if (!adjacentCardinalsAtLeastInState(atLeastState)) {
        return false;
    }

    for (int x = -1; x <= 1; x += 2) {
        for (int z = -1; z <= 1; z += 2) {
            const Chunk* adjacent = adjacentChunks.getChunkLocal(glm::ivec3{x, 0, z});
            if (!adjacent || !adjacent->atLeastInState(atLeastState)) {
                return false;
            }
        }
    }
    return true;
}

void Chunk::computeLightValues() {
    for (int x = 0; x < CHUNK_WIDTH; ++x) {
        for (int z = 0; z < CHUNK_WIDTH; ++z) {
            for (int y = CHUNK_HEIGHT - 1; y >= 0; --y) {

            }
        }
    }
}

std::filesystem::path Chunk::getFileName() const {
    return std::format("{}/level/c{}_{}.bin", Config::settings->world.saveFile, coords.x, coords.z);
}

void Chunk::serialize() {
    if (!Config::settings->game.saveChunks) {
        return;
    }

    ser::Object object{};

    serializeDeserialize(object);

    // object.setField("state", static_cast<u8>(state.load()));

    std::vector<u8> savedBlocks{};

    int currentNumber = 0;
    Block currentBlock = blocks[0][0][0];

    ITERATE_CHUNK_YXZ(x, y, z) {
        Block block = blocks[x][y][z];
        if (block != currentBlock || currentNumber == UINT8_MAX + 1) {
            savedBlocks.push_back((currentNumber - 1));
            savedBlocks.push_back(static_cast<u8>(currentBlock));
            currentNumber = 0;
            currentBlock = block;
        }
        currentNumber++;
    }

    if (currentNumber != 0) {
        savedBlocks.push_back((currentNumber - 1));
        savedBlocks.push_back(static_cast<u8>(currentBlock));
    }
    
    object.setField("blocks", ser::List{savedBlocks});

    try {
        ser::serialize(getFileName(), ser::Dynamic{object});
    } catch (ser::Error& ex) {
        Logger::error(std::format("While serializing '{}': {}", getFileName().c_str(), ex.what()));
    }
}

bool Chunk::deserialize() {
    if (!Config::settings->game.loadChunks) {
        return false;
    }

    std::string fileName = getFileName();
    if (!std::filesystem::is_regular_file(fileName)) {
        return false;
    }

    try {
        ser::Object object = ser::deserialize(fileName).get<ser::Object>();

        serializeDeserialize(object);

        // state = static_cast<ChunkState>(object.getField<u8>("state"));

        std::vector<u8>& loadedBlocks = object.getField<ser::List>("blocks").getVector<u8>();

        int index = 0;
        int currentNumber = 0;
        Block currentBlock = Block::air;

        ITERATE_CHUNK_YXZ(x, y, z) {
            if (currentNumber <= 0) {
                currentNumber = loadedBlocks.at(index++) + 1;
                int blockId = loadedBlocks.at(index++);

                currentBlock = static_cast<Block>(blockId);
            }

            blocks[x][y][z] = currentBlock;
            currentNumber--;
        }

        dirty = true;

    } catch (ser::Error& ex) {
        Logger::error(std::format("While deserializing '{}': {}", fileName, ex.what()));
        return false;
    }

    return true;
}

void Chunk::serializeDeserialize(ser::Object& object) {
    (void)object;
}

BoundingBox Chunk::getCullBoundingBox() const {
    glm::vec3 worldPosition = coords * chunkSize;
    return BoundingBox{worldPosition, worldPosition + glm::vec3{chunkSize}};
}

bool Chunk::blockInChunk(glm::ivec3 local) {
    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}


// ChunkCache


ChunkCache::ChunkCache(glm::ivec3 centerPosition, Chunk* centerChunk)
    : centerPosition{centerPosition}
{
    chunks.fill(nullptr);
    chunks[getIndexLocal(glm::ivec3{0})] = centerChunk;
}

void ChunkCache::setChunk(glm::ivec3 localChunkPosition, Chunk* chunk) {
    chunks[getIndexLocal(localChunkPosition)] = chunk;
}

bool ChunkCache::inCacheGlobal(glm::ivec3 globalChunkPosition) const {
    return inCacheLocal(getLocalGlobal(globalChunkPosition));
}

const Chunk* ChunkCache::getChunkLocal(glm::ivec3 localChunkPosition) const {
    if (!inCacheLocal(localChunkPosition)) {
        return nullptr;
    }
    return chunks[getIndexLocal(localChunkPosition)];
}

Chunk* ChunkCache::getChunkLocal(glm::ivec3 localChunkPosition) {
    if (!inCacheLocal(localChunkPosition)) {
        return nullptr;
    }
    return chunks[getIndexLocal(localChunkPosition)];
}

const Chunk* ChunkCache::getChunkGlobal(glm::ivec3 globalChunkPosition) const {
    return getChunkLocal(getLocalGlobal(globalChunkPosition));
}

Chunk* ChunkCache::getChunkGlobal(glm::ivec3 globalChunkPosition) {
    return getChunkLocal(getLocalGlobal(globalChunkPosition));
}

const Chunk* ChunkCache::getChunkDirection(Direction direction) const {
    return getChunkLocal(directionToPoint(direction));
}

Chunk* ChunkCache::getChunkDirection(Direction direction) {
    return getChunkLocal(directionToPoint(direction));
}

Block ChunkCache::getBlockGlobal(glm::ivec3 globalBlockPosition) const {
    return getBlockLocal(getLocalGlobal(globalBlockPosition));
}

Block ChunkCache::getBlockLocal(glm::ivec3 localBlockPosition) const {
    if (localBlockPosition.y >= CHUNK_HEIGHT || localBlockPosition.y < 0) {
        return Block::air;
    }
    const Chunk* chunk = getChunkLocal(worldToChunk(localBlockPosition));
    if (chunk == nullptr) {
        return Block::barrier;
    }
    return chunk->getBlockRaw(worldToLocal(localBlockPosition));
}
