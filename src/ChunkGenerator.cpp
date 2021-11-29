
#include "ChunkGenerator.h"
#include "Components.h"
//#include <glad.h>
#include <iostream>
#include <queue>

ChunkGenerator::ChunkGenerator(int seed, entt::registry& registry)
    : m_Seed(seed), m_Registry(registry)
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

ChunkComponent ChunkGenerator::createChunkComponent(glm::vec3 chunkPos) {
    ChunkComponent chunkComp;
    chunkComp.hasChanged = true; // toggle for mesh update/construction
    chunkComp.biomeMap = generateBiomeMap(chunkPos);
    chunkComp.blocks = createChunkBlocks(chunkPos, chunkComp.biomeMap);
    createLightMap(chunkComp);

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

//    chunkMap[std::make_pair((int)chunkPos.x, (int)chunkPos.z)] = &e_Chunk;
    chunkMap.insert({std::make_pair(chunkPos.x, chunkPos.z), e_Chunk});

    updateNeighbors(e_Chunk, chunkPos); // place entity in neighbors list of adjacent chunks
}

void ChunkGenerator::destroyChunk(const entt::entity& e_Chunk, glm::vec3 chunkPos)
{
    std::map<std::pair<int, int>, entt::entity>::iterator iter = chunkMap.find(std::make_pair(chunkPos.x, chunkPos.z));
    if (iter == chunkMap.end())
    {
        std::cout << "ChunkGenerator::WARNING - chunkMap entry deletion requested, but no entry found." << std::endl;
        return;
    }

    chunkMap.erase(iter); // remove from lookup search tree
    // TODO: save to disk to support changing environment
    MeshComponent& meshComp = m_Registry.get<MeshComponent>(e_Chunk);
    glDeleteBuffers(1, &meshComp.blockVBO); // can't simply include in MeshComp destructor (swap&pop double destruct)
    updateNeighbors(entt::null, chunkPos); // update list of neighbors for surrounding chunk entites
    m_Registry.destroy(e_Chunk); // delete entity from registry
}

void ChunkGenerator::updateNeighbors(const entt::entity& e_Chunk, glm::vec3 chunkPos)
{
    // accepts chunk being created or destroyed and its position
    // updates surrounding chunks' neighbors array to reflect creation/destruction

    // x defines WEST-EAST axis, z defines NORTH-SOUTH axis
    // goes EAST --> update WEST, goes SOUTH --> update NORTH
    // [CW, 0] [0, -CW] [-CW, 0] [0, CW]
    std::vector<int> dirDist {CHUNK_WIDTH, 0, -CHUNK_WIDTH, 0, CHUNK_WIDTH};
    std::vector<Direction> oppositeDirIdx {WEST, NORTH, EAST, SOUTH};

    // iterate over surrounding 4 chunks
    for (int i = 0; i < 4; i++)
    {
        // identify x,z coordinates of neighbor chunk
        std::pair<int, int> neighborPos(chunkPos.x + dirDist[i], chunkPos.z + dirDist[i + 1]);
        Direction dirTowardUpdatedChunk = oppositeDirIdx[i]; // e_Chunk is in this direction

        // check if neighbor chunk exists in memory
        auto chunkIter = chunkMap.find(neighborPos);
        if (chunkIter == chunkMap.end()) // skip iteration if no neighbor exists in memory
            continue;

        // update neighbor array corresponding to neighboring chunk entity
        entt::entity e_AdjChunk = chunkIter->second;
        std::vector<entt::entity>& adjChunkNeighbors = m_Registry.get<ChunkComponent>(e_AdjChunk).neighborEntities;
        adjChunkNeighbors[dirTowardUpdatedChunk] = e_Chunk;
    }
}

void ChunkGenerator::createLightMap(ChunkComponent& chunkComp) {
    // TODO: include entt::entity and update bounds checking to support cross-voxel light flooding
    struct lightNode {
        int x, y, z, lightLevel;
        lightNode(int x, int y, int z, int lightLevel)
            : x(x), y(y), z(z), lightLevel(lightLevel) {}
    };

    std::queue<lightNode> q;

    // flood fill sunlight from the top of the chunk
    // all voxels with direct vertical access to top of chunk are source nodes
    for (int x = 0; x < CHUNK_WIDTH; x++)
        for (int z = 0; z < CHUNK_WIDTH; z++)
        {
            int y = CHUNK_HEIGHT - 1;
            // TODO: include OR transparent to support transparent blocks
            //
            while (y >= 0 && chunkComp.blockAt(x, y, z) == AIR)
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

            if (neighborX < 0 || neighborX >= CHUNK_WIDTH || neighborY < 0 || neighborY >= CHUNK_WIDTH
                || neighborZ < 0 || neighborZ >= CHUNK_HEIGHT)
                continue; // out of bounds
            if (chunkComp.blockAt(neighborX, neighborY, neighborZ) != AIR) // TODO: update for transparent blocks
                continue; // no light within opaque blocks

            // flood fill adjacent blocks (if lower light level than flood)
            if (chunkComp.getSunlight(neighborX, neighborY, neighborZ) < curNode.lightLevel - 1) {
                chunkComp.setSunlight(neighborX, neighborY, neighborZ, curNode.lightLevel - 1);
                q.emplace(neighborX, neighborY, neighborZ, curNode.lightLevel - 1);
            }
        }

    }

}

