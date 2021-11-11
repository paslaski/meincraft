
#include <entt.hpp>

#include "ChunkLoaderSystem.h"

#include "Chunk.h"
#include "Block.h"
#include "Components.h"

ChunkLoaderSystem::ChunkLoaderSystem(entt::registry& registry)
    : chunkGenerator(5271998, registry)
{

}

ChunkLoaderSystem::~ChunkLoaderSystem()
{}

//void ChunkLoaderSystem::createChunk(entt::registry& registry)
//{
//    entt::entity chunk = registry.create(); // populate registry with chunk entity
//
//    // create position component
//    glm::vec3 pos(0.0f, 0.0f, 0.0f);
//    registry.emplace<PositionComponent>(chunk, pos);
//
//    // create block component
//    std::vector<BlockType> blocks(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, BlockType::GRASS);
//    blocks[0] = BlockType::COBBLESTONE;
//    blocks[CHUNK_WIDTH - 1] = BlockType::SAND;
//    blocks[CHUNK_WIDTH - 2] = AIR;
//    blocks[CHUNK_WIDTH * CHUNK_WIDTH - 1] = BlockType::STONE;
//    for (int i = 3; i < 8; i++)
//        for (int j = 5; j < 7; j++)
//            blocks[i + j * CHUNK_WIDTH] = LADDER;
//
//    for (int j = 8; j < 12; j++)
//        for (int k = 3; k < 9; k++)
//            blocks[j * CHUNK_WIDTH + k * CHUNK_WIDTH * CHUNK_WIDTH] = LADDER;
//
//    // bool hasChanged, std::vector<BlockType> blocks
//    registry.emplace<ChunkComponent>(chunk, true, blocks);
//
//    // create mesh component & associated OpenGL buffer object (uncopyable)
//    unsigned int chunkVBO;
//    glGenBuffers(1, &chunkVBO); // future: ensure gen buffer when loaded off disk/from compressed
//    std::vector<texArrayVertex> chunkVertices;
//    // bool mustUpdateBuffer, unsigned int blockVBO, std::vector<texArrayVertex> chunkVertices;
//    // std::move for OpenGL object
//    registry.emplace<MeshComponent>(chunk, true, std::move(chunkVBO), chunkVertices);
//}

void ChunkLoaderSystem::update(entt::registry &registry) {
    // does nothing for now

    // query for player location

    // iterate through entities with chunkComponent in registry
        // UNLOAD_DISTANCE: delete out-of-range
        // LOAD_DISTANCE: generate/load in-range if missing

}