
#include "ChunkGenerator.h"
#include "Components.h"
//#include <glad.h>
#include <iostream>
#include <queue>

ChunkGenerator::ChunkGenerator(int seed, entt::registry& registry)
    : m_Seed(seed), m_Registry(registry), m_BlockPool(BlockPool::getPoolInstance())
{
    terrainBaseNoise.SetSeed(m_Seed);
    terrainBaseNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    terrainBaseNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    terrainBaseNoise.SetFrequency(0.00522f);
    terrainBaseNoise.SetFractalOctaves(16);
    // terrainBaseNoise.SetFractalGain();
    terrainBaseNoise.SetFractalLacunarity(1.0f);

    biomeTopNoise.SetSeed(m_Seed);
    biomeTopNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    biomeTopNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    biomeTopNoise.SetFrequency(0.00781f);
    biomeTopNoise.SetFractalOctaves(4);
    // biomeTopNoise.SetFractalGain();
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

void ChunkGenerator::createChunkComponent(const entt::entity& e_Chunk, glm::vec3 chunkPos) {
    ChunkComponent chunkComp;
    chunkComp.biomeMap = generateBiomeMap(chunkPos);
    createChunkBlocks(chunkComp, chunkPos, chunkComp.biomeMap);
    updateLightMap(chunkComp);
    m_Registry.emplace<ChunkComponent>(e_Chunk, chunkComp);
}

void ChunkGenerator::createChunkBlocks(ChunkComponent& chunkComp, const glm::vec3& chunkPos,
                                       const std::vector<BiomeType>& biomeMap)
{
    std::vector<const Block*> blocks(CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_WIDTH, m_BlockPool.getBlockPtr(AIR));
    std::vector<int> baseHeightmap = generateBaseHeightmap(chunkPos);
    std::vector<int> biomeTop = generateBiomeTopHeightmap(chunkPos);

    for (int z = 0; z < CHUNK_WIDTH; z++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            int y = 0;
            for (; y < baseHeightmap[x + z * CHUNK_WIDTH]; y++) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = m_BlockPool.getBlockPtr(STONE);
            }

            BlockType biomeBlock = biomeTopBlockLookup(biomeMap[x + z * CHUNK_WIDTH]);
            for (int topperHeight = 0; topperHeight < biomeTop[x + z * CHUNK_WIDTH]; topperHeight++) {
                blocks[x + z * CHUNK_WIDTH + (y + topperHeight) * CHUNK_WIDTH * CHUNK_WIDTH]
                                                        = m_BlockPool.getBlockPtr(biomeBlock);
            }
        }
    }

    chunkComp.setBlock(blocks);
}

BiomeType ChunkGenerator::biomeLookup(float temperature, float precipitation) {
    if (temperature > 0.8f && precipitation < 0.2f)
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

// TODO: multithreaded block/light + mesh creation
// populate m_Registry with entity, create components of chunk, and return chunk entity
const entt::entity ChunkGenerator::generateChunk(glm::vec3 chunkPos) {
    const entt::entity e_Chunk = m_Registry.create();
    m_Registry.emplace<PositionComponent>(e_Chunk, chunkPos);

    // bool hasChanged, std::vector<BlockType> blocks
    createChunkComponent(e_Chunk, chunkPos);

    // create mesh component & associated OpenGL buffer object (uncopyable)
    unsigned int chunkVBO;
    glGenBuffers(1, &chunkVBO); // future: ensure gen buffer when loaded off disk/from compressed
    std::vector<texArrayVertex> chunkVertices; // uninitialized, marked for update
    // bool mustUpdateBuffer, unsigned int blockVBO, std::vector<texArrayVertex> chunkVertices;
    // std::move for OpenGL object
    m_Registry.emplace<MeshComponent>(e_Chunk, true, std::move(chunkVBO), chunkVertices);

    return e_Chunk;
}

void ChunkGenerator::updateLightMap(ChunkComponent& chunkComp) {
    // TODO: include entt::entity and update bounds checking to support cross-voxel light flooding
    struct lightNode {
        int x, y, z, lightLevel;
        lightNode(int x, int y, int z, int lightLevel)
            : x(x), y(y), z(z), lightLevel(lightLevel) {}
    };

    chunkComp.clearLightMap();
    std::queue<lightNode> q;

    // flood fill sunlight from the top of the chunk
    // all voxels with direct vertical access to top of chunk are source nodes
    for (int x = 0; x < CHUNK_WIDTH; x++)
        for (int z = 0; z < CHUNK_WIDTH; z++)
        {
            int y = CHUNK_HEIGHT - 1;
            // propagates through transparent/air blocks
            while (y >= 0 && chunkComp.blockAt(x, y, z)->isTransparent())
            {
                chunkComp.setSunlight(x, y, z, 15); // max light value = 15
                q.emplace(x, y, z, 15);
                y--;
            }
        }

    // BFS to flood fill sunlight
    while (!q.empty())
    {
        lightNode curNode = q.front();
        q.pop();

        for (int dir = 0; dir < 6; dir++) // iterate through WEST EAST SOUTH NORTH DOWN UP
        {
            int neighborX = curNode.x + deltaXByDir[dir]; // coordinates of adjacent block
            int neighborY = curNode.y + deltaYByDir[dir];
            int neighborZ = curNode.z + deltaZByDir[dir];
            if (neighborX < 0 || neighborX >= CHUNK_WIDTH || neighborY < 0 || neighborY >= CHUNK_HEIGHT
                || neighborZ < 0 || neighborZ >= CHUNK_WIDTH)
                continue; // out of bounds
            if (not chunkComp.blockAt(neighborX, neighborY, neighborZ)->isTransparent())
                continue; // no light within opaque blocks

            // flood fill adjacent blocks (if lower light level than flood)

            if (chunkComp.getSunlight(neighborX, neighborY, neighborZ) < curNode.lightLevel - 1) {
                chunkComp.setSunlight(neighborX, neighborY, neighborZ, curNode.lightLevel - 1); // !!!!!????? print idk
                q.emplace(neighborX, neighborY, neighborZ, curNode.lightLevel - 1);
            }
        }

    }

}


