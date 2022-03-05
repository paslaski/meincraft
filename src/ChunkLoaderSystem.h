#pragma once

#include <entt/entt.hpp>
#include <utility>
#include "ChunkGenerator.h"

class ChunkMeshingSystem;

class ChunkLoaderSystem {
public:
    // eventually might need to accept path to world disk storage
    ChunkLoaderSystem(entt::registry& registry, const int seed);
    ~ChunkLoaderSystem();

    void update(entt::registry& registry);
    const Block* blockAt(glm::vec3 pos);

private:
    entt::registry& m_Registry;
    ChunkMapComponent& m_ChunkMap;
    ChunkGenerator m_ChunkGenerator;

    // refactor + expand: loadChunk (generate vs. disk), unloadChunk
    void destroyChunk(const entt::entity& e_Chunk, glm::vec3 chunkPos);
    ChunkMapComponent& createChunkMap(entt::registry& registry);

    // load all chunks that are <= $chunkLoadDistance chunks from player
    const int chunkLoadDistance = 10;
    // any chunks more than $chunkUnloadDistance from player should be removed from memory
    const int chunkUnloadDistance = 12;
};
