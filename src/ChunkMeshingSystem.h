#pragma once

#include <entt.hpp>
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "block.h"
#include "texture.h"

class ChunkMeshingSystem {
public:
    ChunkMeshingSystem();
    ~ChunkMeshingSystem();

    void update(entt::registry& registry);
private:
    void constructMesh(entt::entity& chunk, entt::registry& registry);
    void greedyMesh(entt::entity& chunk, entt::registry& registry);
    void quad(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, BlockType block,
                         Direction dir, std::vector<texArrayVertex>& vertices);
};
