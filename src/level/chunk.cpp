#include <filesystem>
#include <format>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "engine/logger.hpp"
#include "chunk.hpp"
#include "serialization/serialize.hpp"
#include "world.hpp"
#include "chunk.hpp"
#include "worldgen.hpp"
#include "engine/logger.hpp"
#include "graphics/graphics.hpp"


Chunk::Chunk(World* world, glm::ivec3 coords)
    : world{world},
    coords{coords},
    loaded{true}
{
    Logger::assertion(world);

    for (int i = 0; i < DIRECTION_CARDINAL_COUNT; i += 1) {
        Chunk* adjacent = world->getChunk(coords + directionToPoint(static_cast<Direction>(i)));
        adjacentChunks[i] = adjacent;
        if (adjacent != NULL) {
            adjacent->adjacentChunks[invertDirection(static_cast<Direction>(i))] = this;
            adjacent->dirty = true;
        }
    }
}

Chunk::~Chunk() {
    if (loaded) {
        unload();
    }
}

Chunk::Chunk() {}

void Chunk::generateOrLoad() {
    if (deserialize()) {
        Logger::trace(std::format("Chunk {}, {} loaded from file", coords.x, coords.z));
        return;
    }

    generateTerrain(this);
    placeFeatures(this);

    Logger::trace(std::format("Chunk {}, {} generated", coords.x, coords.z));
}

void Chunk::unload() {
    loaded = false;

    for (int i = 0; i < DIRECTION_CARDINAL_COUNT; ++i) {
        Chunk* adjacent = adjacentChunks[i];

        if (adjacent != nullptr) {
            adjacent->adjacentChunks[invertDirection(static_cast<Direction>(i))] = NULL;
        }

        adjacentChunks[i] = nullptr;
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

    if (!loaded) {
        Logger::error(std::format("Drawing unloaded chunk with vao: {}", mesh.vertexArrayObject.object));
    }

    shader.setUniformMat4("model", transform);
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

    ser::serialize(getFileName(), ser::Dynamic{object});
}

bool Chunk::deserialize() {
    if (!Config::settings->game.loadChunks) {
        return false;
    }

    std::string fileName = getFileName();
    if (!std::filesystem::is_regular_file(fileName)) {
        return false;
    }

    ser::Object object = ser::deserialize(fileName).get<ser::Object>();

    serializeDeserialize(object);

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
    return true;
}

void Chunk::serializeDeserialize(ser::Object& object) {
    (void)object;
}


bool Chunk::blockInChunk(glm::ivec3 local) {
    return local.x >= 0 && local.x < CHUNK_WIDTH
        && local.y >= 0 && local.y < CHUNK_HEIGHT
        && local.z >= 0 && local.z < CHUNK_WIDTH;
}
