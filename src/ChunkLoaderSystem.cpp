
#include <entt.hpp>

#include "ChunkLoaderSystem.h"

#include "chunk.h"
#include "block.h"
#include "components.h"

ChunkLoaderSystem::ChunkLoaderSystem()
{}

ChunkLoaderSystem::~ChunkLoaderSystem()
{}

void ChunkLoaderSystem::createChunk(entt::registry& registry)
{
    entt::entity chunk = registry.create(); // populate registry with chunk entity

    // create position component
    glm::vec3 pos(0.0f, 0.0f, 0.0f);
    registry.emplace<PositionComponent>(chunk, pos);

    // create block component
    std::vector<BlockType> blocks(CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE, BlockType::GRASS);
    blocks[0] = BlockType::COBBLESTONE;
    blocks[CHUNK_SIZE-1] = BlockType::SAND;
    blocks[CHUNK_SIZE-2] = AIR;
    blocks[CHUNK_SIZE*CHUNK_SIZE-1] = BlockType::STONE;
    // bool hasChanged, std::vector<BlockType> blocks
    registry.emplace<BlockComponent>(chunk, true, blocks);

    // create mesh component & associated OpenGL buffer object (uncopyable)
    unsigned int chunkVBO;
    glGenBuffers(1, &chunkVBO); // future: ensure gen buffer when loaded off disk/from compressed
    std::vector<texArrayVertex> chunkVertices;
    // bool mustUpdateBuffer, unsigned int blockVBO, std::vector<texArrayVertex> chunkVertices;
    // std::move for OpenGL object
    registry.emplace<MeshComponent>(chunk, true, std::move(chunkVBO), chunkVertices);
}

void ChunkLoaderSystem::update(entt::registry &registry) {
    // does nothing for now
}
