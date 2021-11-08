#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <FastNoiseLite/FastNoiseLite.h>
#include <entt.hpp>

#include "Chunk.h"
#include "Block.h"

class ChunkGenerator {
public:
    ChunkGenerator(int seed, entt::registry& registry);
    ~ChunkGenerator();

    std::vector<int> generateHeightmap(glm::vec3 pos);
    void createChunkEntity(std::vector<BlockType>& blocks, glm::vec3 pos);
    void assignBlocks(std::vector<BlockType>& blocks, std::vector<int>& heightmap);
    void generateChunk(glm::vec3 pos);

private:
    const int m_Seed;
    FastNoiseLite m_Noise;
    entt::registry& m_Registry;

    // may not want const in future
    const float frequency;
    const float lacunarity;
    const int octaves;
    const float gain;

    const int minHeight = 6;
    const int maxHeight = CHUNK_HEIGHT - 1;

    const int chunkWidth = CHUNK_WIDTH;
};