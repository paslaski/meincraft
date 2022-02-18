#pragma once

#include <entt/entt.hpp>
#include <utility>
#include "ChunkGenerator.h"

class ChunkLoaderSystem {
public:
    // eventually might need to accept path to world disk storage
    ChunkLoaderSystem(entt::registry& registry);
    ~ChunkLoaderSystem();

    void update(entt::registry& registry);

    // make private after initial testing
    ChunkGenerator chunkGenerator;
private:
    // load all chunks that are <= $chunkLoadDistance chunks from player
    const int chunkLoadDistance = 2;
    // any chunks more than $chunkUnloadDistance from player should be removed from memory
    const int chunkUnloadDistance = 3;
};
