#pragma once

#include <entt/entt.hpp>
#include <glad/glad.h>
#include <glm/vec3.hpp>

#include "Block.h"
#include "Texture.h"
#include "Components.h"

class ChunkMeshingSystem {
public:
    ChunkMeshingSystem();
    ~ChunkMeshingSystem();

    void update(entt::registry& registry);
private:
    void constructMesh(entt::entity& chunk, entt::registry& registry);
    void greedyMesh(const entt::entity& chunk, entt::registry& registry);
    void appendQuad(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 v4, BlockType block,
                         int width, int height,
                         Direction dir, std::vector<texArrayVertex>& vertices,
                         uint8_t lightLevel);

    uint8_t getLightLevelAt(entt::registry& registry, const entt::entity& e_Chunk,
                                                ChunkComponent& chunkComp, const int x, const int y,
                                                const int z);
    float uvCoords[12] = {
            0.0f, 0.0f, // first triangle on face
            1.0f, 0.0f,
            0.0f, 1.0f,
            0.0f, 1.0f, // second triangle on face
            1.0f, 1.0f,
            1.0f, 0.0f
    };
};
