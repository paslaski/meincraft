
#include "ChunkGenerator.h"
#include "Components.h"
//#include <glad.h>
#include <iostream>

ChunkGenerator::ChunkGenerator(int seed, entt::registry& registry)
    : m_Seed(seed), m_Registry(registry), frequency(0.01), octaves(1), lacunarity(2.0), gain(0.5)
{
    terrainBaseNoise.SetSeed(m_Seed);
    terrainBaseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    terrainBaseNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    terrainBaseNoise.SetFrequency(0.00522f);
    terrainBaseNoise.SetFractalOctaves(16);
//    terrainBaseNoise.SetFractalGain();
    terrainBaseNoise.SetFractalLacunarity(1.0f);

    biomeTopNoise.SetSeed(m_Seed);
    biomeTopNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeTopNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    biomeTopNoise.SetFrequency(0.00781f);
    biomeTopNoise.SetFractalOctaves(4);
//    biomeTopNoise.SetFractalGain();
    biomeTopNoise.SetFractalLacunarity(2.333f);

    temperatureNoise.SetSeed(m_Seed);
    temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    temperatureNoise.SetFrequency(0.0008);
    precipitationNoise.SetSeed(m_Seed);
    precipitationNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    precipitationNoise.SetFrequency(0.0008);
}

ChunkGenerator::~ChunkGenerator()
{}

// change blockComponent to chunkComponent, blockAt to blockAt, add biomeMap
// createChunkComponent function invokes generateBaseHeightmap, generateBiomeMap, biomeLookup to create full comp


ChunkComponent ChunkGenerator::createChunkComponent(glm::vec3 chunkPos) {
    ChunkComponent chunkComp;
    chunkComp.hasChanged = true; // toggle for mesh update/construction
    chunkComp.biomeMap = generateBiomeMap(chunkPos);
    chunkComp.blocks = createChunkBlocks(chunkPos, chunkComp.biomeMap);

    return chunkComp;
}

std::vector<BlockType> ChunkGenerator::createChunkBlocks(glm::vec3 chunkPos, std::vector<BiomeType>& biomeMap)
{
    std::vector<BlockType> blocks(CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_WIDTH, AIR);

    std::vector<int> baseHeightmap = generateBaseHeightmap(chunkPos);
    std::vector<int> biomeTop = generateBiomeTopHeightmap(chunkPos);

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            int y = 0;
            for (; y < baseHeightmap[x + z * CHUNK_WIDTH]; y++) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = STONE;
            }

            BlockType biomeBlock = biomeTopBlockLookup(biomeMap[x + z * CHUNK_WIDTH]);
            for (int topperHeight = 0; topperHeight < biomeTop[x + z * CHUNK_WIDTH]; topperHeight++) {
                blocks[x + z * CHUNK_WIDTH + (y + topperHeight) * CHUNK_WIDTH * CHUNK_WIDTH] = biomeBlock;
            }
        }
    }

    return blocks;
}

BiomeType ChunkGenerator::biomeLookup(float temperature, float precipitation) {
    if (temperature > 0.8f && precipitation > 0.8f)
        return SandBiome;

    return GrassBiome;
}

std::vector<BiomeType> ChunkGenerator::generateBiomeMap(glm::vec3 chunkPos) {
    std::vector<BiomeType> biomeMap(CHUNK_WIDTH * CHUNK_WIDTH);
    float temperature;
    float precipitation;
    auto scale = [](float val) { return (val + 1.0f)/2.0f; };

    for (int z = 0; z < CHUNK_WIDTH; z++)
        for (int x = 0; x < CHUNK_WIDTH; x++)
        {
            temperature = scale(temperatureNoise.GetNoise(chunkPos.x + x, chunkPos.z + z));
            precipitation = scale(precipitationNoise.GetNoise(chunkPos.x + x, chunkPos.z + z));
            // also include elevation? idts it changes too fast
            biomeMap[x + z * CHUNK_WIDTH] = biomeLookup(temperature, precipitation);
        }

    return biomeMap;
}

std::vector<int> ChunkGenerator::generateBaseHeightmap(glm::vec3 chunkPos)
{
    std::vector<int> baseHeightmap(CHUNK_WIDTH*CHUNK_WIDTH, 0);

    // lambda scales noise to desired range: [minBaseHeight, maxBaseHeight]
    auto scale = [this](float val) { return static_cast<int>((val + 1.0)*(maxBaseHeight-minBaseHeight)/2.0 + minBaseHeight); };

    for (int z = 0; z < CHUNK_WIDTH; z++)
        for (int x = 0; x < CHUNK_WIDTH; x++)
            baseHeightmap[x + z*CHUNK_WIDTH] =
                    scale(terrainBaseNoise.GetNoise((float) (x + chunkPos.x), (float) (z + chunkPos.z)));

    return baseHeightmap;
}

std::vector<int> ChunkGenerator::generateBiomeTopHeightmap(glm::vec3 chunkPos)
{
    std::vector<int> biomeTopHeightmap(CHUNK_WIDTH*CHUNK_WIDTH, 0);

    // lambda scales noise to desired range: [minBaseHeight, maxBaseHeight]
    auto scale = [this](float val) { return static_cast<int>((val + 1.0)*(maxBiomeHeight-minBiomeHeight)/2.0 + minBiomeHeight); };

    for (int z = 0; z < CHUNK_WIDTH; z++)
        for (int x = 0; x < CHUNK_WIDTH; x++)
            biomeTopHeightmap[x + z*CHUNK_WIDTH] =
                    scale(biomeTopNoise.GetNoise((float) (x + chunkPos.x), (float) (z + chunkPos.z)));

    return biomeTopHeightmap;
}

void ChunkGenerator::generateChunk(glm::vec3 chunkPos) {
    entt::entity e_Chunk = m_Registry.create();
    m_Registry.emplace<PositionComponent>(e_Chunk, chunkPos);

    // bool hasChanged, std::vector<BlockType> blocks
    ChunkComponent chunkComp = createChunkComponent(chunkPos);
    m_Registry.emplace<ChunkComponent>(e_Chunk, chunkComp);

    // create mesh component & associated OpenGL buffer object (uncopyable)
    unsigned int chunkVBO;
    glGenBuffers(1, &chunkVBO); // future: ensure gen buffer when loaded off disk/from compressed
    std::vector<texArrayVertex> chunkVertices; // uninitialized, marked for update
    // bool mustUpdateBuffer, unsigned int blockVBO, std::vector<texArrayVertex> chunkVertices;
    // std::move for OpenGL object
    m_Registry.emplace<MeshComponent>(e_Chunk, true, std::move(chunkVBO), chunkVertices);
}

