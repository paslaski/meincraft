#pragma once

#include <entt.hpp>

class ChunkLoaderSystem {
public:
    // eventually might need to accept path to world disk storage
    ChunkLoaderSystem();
    ~ChunkLoaderSystem();

    void update(entt::registry& registry);

    // make private after initial testing with single chunk instantiated from main/world
    void createChunk(entt::registry& registry);
private:

};
