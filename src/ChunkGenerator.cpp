
#include "ChunkGenerator.h"
#include "Components.h"
//#include <glad.h>
#include <iostream>

ChunkGenerator::ChunkGenerator(int seed, entt::registry& registry)
    : m_Seed(seed), m_Registry(registry), frequency(0.01), octaves(1), lacunarity(2.0), gain(0.5)
{
    m_Noise.SetSeed(m_Seed);
    m_Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
}

ChunkGenerator::~ChunkGenerator()
{}

void ChunkGenerator::generateChunk(glm::vec3 pos) {
    std::vector<BlockType> blocks(CHUNK_WIDTH*CHUNK_HEIGHT*CHUNK_WIDTH, AIR);

    std::vector<int> heightmap = generateHeightmap(pos);

    assignBlocks(blocks, heightmap);

    createChunkEntity(blocks, pos);

}

void ChunkGenerator::createChunkEntity(std::vector<BlockType>& blocks, glm::vec3 pos) {
    entt::entity e_Chunk = m_Registry.create();
    m_Registry.emplace<PositionComponent>(e_Chunk, pos);

    // bool hasChanged, std::vector<BlockType> blocks
    m_Registry.emplace<BlockComponent>(e_Chunk, true, blocks);

    // create mesh component & associated OpenGL buffer object (uncopyable)
    unsigned int chunkVBO;
    glGenBuffers(1, &chunkVBO); // future: ensure gen buffer when loaded off disk/from compressed
    std::vector<texArrayVertex> chunkVertices; // uninitialized, marked for update
    // bool mustUpdateBuffer, unsigned int blockVBO, std::vector<texArrayVertex> chunkVertices;
    // std::move for OpenGL object
    m_Registry.emplace<MeshComponent>(e_Chunk, true, std::move(chunkVBO), chunkVertices);
}

void ChunkGenerator::assignBlocks(std::vector<BlockType>& blocks, std::vector<int>& heightmap)
{
    for (int z = 0; z < CHUNK_WIDTH; z++)
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            int y = maxHeight;

            while (y > heightmap[x + z*CHUNK_WIDTH]) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = AIR;
                y--;
            }

            if (y >= 0) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = GRASS;
                y--;
            }
            for (int numDirt = 0; y >= 0 && numDirt < 3; numDirt++) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = DIRT;
                y--;
            }
            while (y >= 0) {
                blocks[x + z * CHUNK_WIDTH + y * CHUNK_WIDTH * CHUNK_WIDTH] = STONE;
                y--;
            }
        }
}

std::vector<int> ChunkGenerator::generateHeightmap(glm::vec3 chunkPos) {
    std::vector<int> heightmap(CHUNK_WIDTH*CHUNK_WIDTH, 0);

    auto scale = [this](float val) { return static_cast<int>((val + 1.0)*(maxHeight-minHeight)/2.0 + minHeight); };

    for (int z = 0; z < CHUNK_WIDTH; z++)
    {
        for (int x = 0; x < CHUNK_WIDTH; x++)
        {
            heightmap[x + z*CHUNK_WIDTH] = scale(m_Noise.GetNoise((float) (x + chunkPos.x), (float) (z + chunkPos.z)));
        }
    }

    return heightmap;
}

