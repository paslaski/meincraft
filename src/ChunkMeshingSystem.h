#pragma once

#include <entt.hpp>

class ChunkMeshingSystem {
public:
    ChunkMeshingSystem();
    ~ChunkMeshingSystem();

    void update(entt::registry& registry);
private:
    void constructMesh(entt::entity& chunk, entt::registry& registry);
};
