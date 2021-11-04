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
    void appendQuad(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, BlockType block,
                         int width, int height,
                         Direction dir, std::vector<texArrayVertex>& vertices);
    float uvCoords[12] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
};
