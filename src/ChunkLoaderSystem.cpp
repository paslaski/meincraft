
#include <algorithm>
#include <stdlib.h>
#include <thread>

#include "ChunkLoaderSystem.h"

#include "Chunk.h"
#include "Block.h"
#include "Components.h"
#include "Player.h"

ChunkLoaderSystem::ChunkLoaderSystem(entt::registry& registry, const int seed)
    : m_Registry(registry), m_ChunkMap(createChunkMap(registry)), m_ChunkGenerator(seed, registry, m_ChunkMap)
{}

ChunkLoaderSystem::~ChunkLoaderSystem()
{}

void ChunkLoaderSystem::update(entt::registry& registry) {
    // TODO: maybe dont do this every frame? only if player leaves (cached) last chunk they were in
    // query for player location
    // std::pair<int, int> playerChunk = getPlayerChunkLocation(m_Registry);
    std::pair<int, int> playerChunk = m_ChunkMap.chunkOf(getPlayerPos(registry));

    std::vector<bool> nearbyChunks((2*chunkLoadDistance+1)*(2*chunkLoadDistance+1), false);

    // iterate through entities with chunkComponent in m_Registry
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
            destroyChunk(e_Chunk, chunkPos);
        }
        if (chunkDist <= chunkLoadDistance) // mark as present
        {
            // x + z*loadedChunkBoxSideLength but offset to avoid negative x/z distances
            nearbyChunks[(xDist+chunkLoadDistance) + (zDist+chunkLoadDistance)*(2*chunkLoadDistance+1)] = true;
        }
    }

    // create any chunks missing within chunkLoadDistance
    std::vector<std::thread> chunkGenThreads;
    for (int zOff = -chunkLoadDistance; zOff <= chunkLoadDistance; zOff++)
    {
        for (int xOff = -chunkLoadDistance; xOff <= chunkLoadDistance; xOff++)
        {
            // xOff + zOff*loadedChunkBoxSideLength but offset to avoid negative x/z distances
            if (not nearbyChunks[(xOff+chunkLoadDistance) + (zOff+chunkLoadDistance)*(2*chunkLoadDistance+1)])
            {
                int chunkX = playerChunk.first + xOff * CHUNK_WIDTH;
                int chunkZ = playerChunk.second + zOff * CHUNK_WIDTH;
                glm::vec3 chunkPos = glm::vec3(chunkX, 0, chunkZ);

                const entt::entity e_Chunk = m_ChunkGenerator.generateChunkEntity(chunkPos);
                chunkGenThreads.emplace_back(&ChunkGenerator::createChunkComponent, &m_ChunkGenerator, e_Chunk, chunkPos);
            }
        }
    }
    // safe for now: wait for all threads to complete before advancing to next system
    for (std::thread& t : chunkGenThreads)
        t.join();
}

void ChunkLoaderSystem::destroyChunk(const entt::entity& e_Chunk, glm::vec3 chunkPos)
{
    m_ChunkMap.deleteChunk(std::make_pair(chunkPos.x, chunkPos.z));

    // TODO: save to disk to support changing environment
    MeshComponent& meshComp = m_Registry.get<MeshComponent>(e_Chunk);
    glDeleteBuffers(1, &meshComp.blockVBO); // can't include in MeshComp destructor (entt swap&pop double destruct)
    m_Registry.destroy(e_Chunk); // delete entity from m_Registry
}

ChunkMapComponent& ChunkLoaderSystem::createChunkMap(entt::registry& registry)
{
    entt::entity e_ChunkMap = m_Registry.create();
    m_Registry.emplace<ChunkMapComponent>(e_ChunkMap, registry);
    return m_Registry.get<ChunkMapComponent>(e_ChunkMap);
}

