
//#include <entt/entt.hpp>
#include <algorithm>
#include <stdlib.h>

#include "ChunkLoaderSystem.h"

#include "Chunk.h"
#include "Block.h"
#include "Components.h"
#include "Player.h"

ChunkLoaderSystem::ChunkLoaderSystem(entt::registry& registry)
    : chunkGenerator(5271998, registry)
{

}

ChunkLoaderSystem::~ChunkLoaderSystem()
{}

void ChunkLoaderSystem::update(entt::registry& registry) {
    // TODO: maybe dont do this every frame? only if player leaves (cached) last chunk they were in
    // query for player location
//    std::pair<int, int> playerChunk = getPlayerChunkLocation(registry);
    std::pair<int, int> playerChunk = chunkOf(getPlayerPos(registry));

    std::vector<bool> nearbyChunks((2*chunkLoadDistance+1)*(2*chunkLoadDistance+1), false);

    // iterate through entities with chunkComponent in registry
        // UNLOAD_DISTANCE: delete out-of-range
        // LOAD_DISTANCE: generate/load in-range if missing
    auto chunkView = registry.view<ChunkComponent>();
    for (const auto& e_Chunk : chunkView)
    {
        glm::vec3& chunkPos = registry.get<PositionComponent>(e_Chunk).pos;
        int xDist = ((int)chunkPos.x - playerChunk.first) / CHUNK_WIDTH;
        int zDist = ((int)chunkPos.z - playerChunk.second) / CHUNK_WIDTH;
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
                int chunkX = playerChunk.first + xOff * CHUNK_WIDTH;
                int chunkZ = playerChunk.second + zOff * CHUNK_WIDTH;
                chunkGenerator.generateChunk(glm::vec3(chunkX, 0, chunkZ));
            }
        }
    }
    // TODO: destroy VBO in OpenGL from component destructors
}