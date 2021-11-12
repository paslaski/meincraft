
#include <entt.hpp>
#include <algorithm>
#include <stdlib.h>

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

void ChunkLoaderSystem::update(entt::registry& registry) {
    // TODO: maybe dont do this every frame?
    // query for player location
    std::pair<int, int> playerPos = getPlayerChunkLocation(registry);

    std::vector<bool> nearbyChunks((2*chunkLoadDistance+1)*(2*chunkLoadDistance+1), false);

    // iterate through entities with chunkComponent in registry
        // UNLOAD_DISTANCE: delete out-of-range
        // LOAD_DISTANCE: generate/load in-range if missing
    auto chunkView = registry.view<ChunkComponent>();
    for (const auto& e_Chunk : chunkView)
    {
        glm::vec3& chunkPos = registry.get<PositionComponent>(e_Chunk).pos;
        int xDist = ((int)chunkPos.x - playerPos.first)/CHUNK_WIDTH;
        int zDist = ((int)chunkPos.z - playerPos.second)/CHUNK_WIDTH;
        int chunkDist = std::max(std::abs(xDist), std::abs(zDist));

        if (chunkDist >= chunkUnloadDistance) // delete from memory
        {
            chunkGenerator.destroyChunk(e_Chunk, chunkPos);
        }
        if (chunkDist <= chunkLoadDistance) // mark as present
        {
            // x + z*loadedChunkBoxSideLength but offset to avoid negative x/z distances
            nearbyChunks[(xDist+chunkLoadDistance) + (zDist+chunkLoadDistance)*(2*chunkLoadDistance+1)] = true;
        }
    }

    // create any chunks missing within chunkLoadDistance
    for (int zOff = -chunkLoadDistance; zOff <= chunkLoadDistance; zOff++)
    {
        for (int xOff = -chunkLoadDistance; xOff <= chunkLoadDistance; xOff++)
        {
            // xOff + zOff*loadedChunkBoxSideLength but offset to avoid negative x/z distances
            if (!nearbyChunks[(xOff+chunkLoadDistance) + (zOff+chunkLoadDistance)*(2*chunkLoadDistance+1)])
            {
                int chunkX = playerPos.first + xOff*CHUNK_WIDTH;
                int chunkZ = playerPos.second + zOff*CHUNK_WIDTH;
                chunkGenerator.generateChunk(glm::vec3(chunkX, 0, chunkZ));
            }
        }
    }
    // TODO: destroy VBO in OpenGL from component destructors
}

std::pair<int, int> ChunkLoaderSystem::getPlayerChunkLocation(entt::registry& registry)
{
    // there should only ever be one player, so return first value found
    auto playerView = registry.view<CameraComponent>();
    for (auto& e_Player : playerView)
    {
        glm::vec3& playerPos = registry.get<CameraComponent>(e_Player).camera->Position;

        // identify current chunk position (classified by minimum x,z)
        int xChunk = (int) playerPos.x / CHUNK_WIDTH * CHUNK_WIDTH;
        int zChunk = (int) playerPos.z / CHUNK_WIDTH * CHUNK_WIDTH;
        // above statement maps to upper right corner (instead of bottom left) for negative values
        if (playerPos.x < 0) xChunk -= CHUNK_WIDTH;
        if (playerPos.z < 0) zChunk -= CHUNK_WIDTH;

        return std::make_pair(xChunk, zChunk);
    }
}