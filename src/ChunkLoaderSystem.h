#pragma once

#include <entt.hpp>
#include "ChunkGenerator.h"

class ChunkLoaderSystem {
public:
    // eventually might need to accept path to world disk storage
    ChunkLoaderSystem(entt::registry& registry);
    ~ChunkLoaderSystem();

    void update(entt::registry& registry);

    // make private after initial testing with single chunk instantiated from main/world
    void createChunk(entt::registry& registry);
    ChunkGenerator chunkGenerator;
private:

};
