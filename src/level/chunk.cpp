#include <filesystem>
#include <format>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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

    for (int x = -1; x <= 1; ++x) {
        for (int z = -1; z <= 1; ++z) {
            if (x == 0 && z == 0) {
                continue;
            }

            glm::ivec3 localChunkPosition{x, 0, z};
            Chunk* adjacent = world->getChunk(coords + localChunkPosition);
            adjacentChunks.setChunk(localChunkPosition, adjacent);
            if (adjacent != nullptr) {
                adjacent->adjacentChunks.setChunk(-localChunkPosition, this);
                adjacent->dirty = true;
            }
        }
    }
}

Chunk::~Chunk() {
    if (state == ChunkState::loaded) {
        unload();
    }
}

Chunk::Chunk() {}

void Chunk::generateOrLoad() {
    if (deserialize()) {
        Logger::trace(std::format("Chunk {}, {} loaded from file", coords.x, coords.z));
        state = ChunkState::loaded;
        return;
    }

    generateTerrain(this);
    placeFeatures(this);

    
    Logger::trace(std::format("Chunk {}, {} generated", coords.x, coords.z));
    state = ChunkState::loaded;
}

void Chunk::unload() {
    state = ChunkState::unloaded;

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

    serialize();
    Logger::trace(std::format("Chunk {}, {} saved", coords.x, coords.z));
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

void Chunk::markDirty(glm::ivec3 local) {
    Logger::assertion(blockInChunk(local));

    dirty = true;

#ifdef USE_IGNORED
    ignored[local.y] |= 1 << local.x;
#endif
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
    //transform = glm::scale(transform, CHUNK_SIZE);
    //transform = glm::translate(transform, pointToVector3(chunk->coords));

    if (state != ChunkState::loaded) {
        Logger::error(std::format("Drawing unloaded chunk with vao: {}", mesh.vertexArrayObject.object));
    }

    shader.setModel(transform);
    mesh.draw();

    if (world->showChunkBorders) {
        wireframeEnable();
        //DrawCubeWiresV(Vector3Multiply(Vector3AddValue(pointToVector3(chunk->coords), 0.5f), (Vector3)CHUNK_SIZE), (Vector3)CHUNK_SIZE, WHITE);
        wireframeDisable();
    }
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

    object.setField("state", static_cast<u8>(state.load()));

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

        state = static_cast<ChunkState>(object.getField<u8>("state"));

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

Block ChunkCache::getBlockRawGlobal(glm::ivec3 globalBlockPosition) const {
    if (globalBlockPosition.y >= CHUNK_HEIGHT || globalBlockPosition.y < 0) {
        return Block::air;
    }
    const Chunk* chunk = getChunkGlobal(worldToChunk(globalBlockPosition));
    if (chunk == nullptr) {
        return Block::barrier;
    }
    return chunk->getBlockRaw(worldToLocal(globalBlockPosition));
}
