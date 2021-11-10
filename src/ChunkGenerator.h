#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <FastNoiseLite/FastNoiseLite.h>
#include <entt.hpp>

#include "Chunk.h"
#include "Block.h"
#include "Components.h"
#include "Biome.h"

class ChunkGenerator {
public:
    ChunkGenerator(int seed, entt::registry& registry);
    ~ChunkGenerator();

    std::vector<int> generateHeightmap(glm::vec3 pos);
    void createChunkEntity(std::vector<BlockType>& blocks, glm::vec3 chunkPos);
    void assignBlocks(std::vector<BlockType>& blocks, std::vector<int>& heightmap);
    void generateChunk(glm::vec3 chunkPos);

    ChunkComponent createChunkComponent(glm::vec3 chunkPos);
    std::vector<BlockType> createChunkBlocks(glm::vec3 chunkPos, std::vector<BiomeType>& biomeMap);
    std::vector<int> generateBaseHeightmap(glm::vec3 chunkPos);
    std::vector<int> generateBiomeTopHeightmap(glm::vec3 chunkPos);
    // how to store biome? pointer? to singleton? enum?
    std::vector<BiomeType> generateBiomeMap(glm::vec3 chunkPos);
    BiomeType biomeLookup(float temperature, float precipitation);
    void generateFlora(std::vector<BlockType> blocks);

private:
    const int m_Seed;
    FastNoiseLite m_Noise;
    entt::registry& m_Registry;

    FastNoiseLite terrainBaseNoise;
    FastNoiseLite biomeTopNoise;
    FastNoiseLite temperatureNoise;
    FastNoiseLite precipitationNoise;

    // may not want const in future
    const float frequency;
    const float lacunarity;
    const int octaves;
    const float gain;

    const int minHeight = 6;
    const int maxHeight = CHUNK_HEIGHT - 1;

    // maxBase + maxBiome + anything on top must be less than CHUNK_HEIGHT

    const int minBaseHeight = 60; // bounds stone
    const int maxBaseHeight = 115;

    const int minBiomeHeight = 2; // bounds grass/sand/snow on top of stone
    const int maxBiomeHeight = 7;

};