#pragma once

#include <vector>

#include <glm/vec3.hpp>
#include <FastNoiseLite/FastNoiseLite.h>
#include <entt.hpp>
#include <memory>

#include "Chunk.h"
#include "Block.h"
#include "Components.h"
#include "Biome.h"

class ChunkGenerator {
public:
    ChunkGenerator(int seed, entt::registry& registry);
    ~ChunkGenerator();

    void generateChunk(glm::vec3 chunkPos);
    void destroyChunk(const entt::entity& e_Chunk, glm::vec3 chunkPos);

private:
    const int m_Seed;
    entt::registry& m_Registry;

    std::map<std::pair<int, int>, entt::entity> chunkMap;

    ChunkComponent createChunkComponent(glm::vec3 chunkPos);
    std::vector<BlockType> createChunkBlocks(glm::vec3 chunkPos, std::vector<BiomeType>& biomeMap);
    std::vector<int> generateBaseHeightmap(glm::vec3 chunkPos);
    std::vector<int> generateBiomeTopHeightmap(glm::vec3 chunkPos);
    // how to store biome? pointer? to singleton? enum?
    std::vector<BiomeType> generateBiomeMap(glm::vec3 chunkPos);
    BiomeType biomeLookup(float temperature, float precipitation);
//    void generateFlora(std::vector<BlockType> blocks);
    void updateNeighbors(const entt::entity& e_Chunk, glm::vec3 chunkPos);
    void createLightMap(ChunkComponent& chunkComp);

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