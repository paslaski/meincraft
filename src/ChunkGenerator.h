#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <FastNoiseLite/FastNoiseLite.h>
#include <entt/entt.hpp>
#include <memory>

#include "Chunk.h"
#include "Block.h"
#include "Components.h"
#include "Biome.h"
#include "BlockPool.h"

class ChunkGenerator {
public:
    ChunkGenerator(int seed, entt::registry& registry);
    ~ChunkGenerator();

    const entt::entity generateChunk(glm::vec3 chunkPos);
    static void updateLightMap(ChunkComponent& chunkComp);

private:
    const int m_Seed;
    entt::registry& m_Registry;
    const BlockPool& m_BlockPool;

    void createChunkComponent(const entt::entity& e_Chunk, glm::vec3 chunkPos);
    void createChunkBlocks(ChunkComponent& chunkComp, const glm::vec3& chunkPos, const std::vector<BiomeType>& biomeMap);
    std::vector<int> generateBaseHeightmap(glm::vec3 chunkPos);
    std::vector<int> generateBiomeTopHeightmap(glm::vec3 chunkPos);
    // how to store biome? pointer? to singleton? enum?
    std::vector<BiomeType> generateBiomeMap(glm::vec3 chunkPos);
    BiomeType biomeLookup(float temperature, float precipitation);
//    void generateFlora(std::vector<BlockType> blocks);

    FastNoiseLite terrainBaseNoise;
    FastNoiseLite biomeTopNoise;
    FastNoiseLite temperatureNoise;
    FastNoiseLite precipitationNoise;

    // maxBase + maxBiome + anything on top must be less than CHUNK_HEIGHT
    const int minBaseHeight = 60; // bounds stone
    const int maxBaseHeight = 115;

    const int minBiomeHeight = 2; // bounds grass/sand/snow on top of stone
    const int maxBiomeHeight = 7;

};