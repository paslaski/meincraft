#pragma once

#include <string>
#include <memory>
#include <glm/vec3.hpp>
#include "Block.h"
#include "Chunk.h"
#include "Texture.h"
#include "Biome.h"
#include "Camera.h"
#include "BlockPool.h"

struct PositionComponent
{
    // eventually change to array representation with [xPos1 yPos1 zPos1 t1Coord1 t2Coord1...]
    glm::vec3 pos;
};

struct ChunkComponent
{
private:
    bool changed; // useful to determine if new meshes should be generated
    std::vector<const Block*> blocks; // = std::vector<BlockType>(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, AIR);

public:
    bool hasChanged() const {
        return changed;
    }

    void markChangesResolved() {
        changed = false;
    }
    // TODO: use getter const {} and setter method, encapsulate hasChanged (private, public getter)

    // "points" to neighboring chunks, but not necessary to use pointers as entities are just integers
    // convention: indexed with [NORTH, SOUTH, WEST, EAST] per enum
    std::vector<entt::entity> neighborEntities {entt::null, entt::null, entt::null, entt::null};

    // include array of 8 bit enums corresponding to block types
    // classifies biome for each x,z index
    std::vector<BiomeType> biomeMap; // = std::vector<BiomeType>(CHUNK_WIDTH * CHUNK_WIDTH, GrassBiome);

    // reference to pointer to constant block
    const Block* blockAt(int x, int y, int z) const {
        return blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)];
    }
    const Block* blockAt(const glm::ivec3& blockPos) const {
        return blocks[blockPos.x + (blockPos.z * CHUNK_WIDTH) + (blockPos.y * CHUNK_WIDTH * CHUNK_WIDTH)];
    }

    void setBlock(std::vector<const Block*>& blockArr) {
        blocks = blockArr;
        changed = true;
    }
    void setBlock(const glm::ivec3& blockPos, const BlockType type) {
        if (blockAt(blockPos)->typeOf() == type) // avoid meshing/lighting again if no changes
            return;

        blocks[blockPos.x + (blockPos.z * CHUNK_WIDTH) + (blockPos.y * CHUNK_WIDTH * CHUNK_WIDTH)]
                = BlockPool::getPoolInstance().getBlockPtr(type);
        changed = true; // note updated block state
    }

    // sunlight corresponds to the bits 0000XXXX
    // torchlight corresponds to bits XXXX0000
    std::vector<uint8_t> lightMap = std::vector<uint8_t>(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, 0);
    int getSunlight(int x, int y, int z) {
        return lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)] & 0xF;
    }
    void setSunlight(int x, int y, int z, int val) {
        lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)]
                = (lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)] & 0xF0) | val;
    }
    int getTorchlight(int x, int y, int z) {
        return (lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)] >> 4) & 0xF;
    }
    void setTorchlight(int x, int y, int z, int val) {
        lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)]
                = (lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)] & 0xF) | (val << 4);
    }
    uint8_t lightAt(int x, int y, int z) {
        return lightMap[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)];
    }
};

// destructor being called a lot... does this have to do with initializing each entity's mesh component / overwriting?
struct MeshComponent // can add more VBOs for different rendering processes
{
    // shader, texture, VAO stored in RenderSystem for each VBO
    bool mustUpdateBuffer; // if vertex data different, must update buffer for GPU
    unsigned int blockVBO; // VBO for block
    std::vector<texArrayVertex> chunkVertices; // vertex data

    // destructor needed? gl objects/programs
    // disable copying and enable moving
    MeshComponent(bool b, unsigned int vbo, std::vector<texArrayVertex> vertices)
        : mustUpdateBuffer(b), blockVBO(std::move(vbo)), chunkVertices(vertices) {};
    MeshComponent(const MeshComponent&) = delete;
    // swap & pop means destructor called twice --> must call glDeleteBuffers outside
    MeshComponent operator=(const MeshComponent&) = delete;
    MeshComponent(MeshComponent&&) = default;
    MeshComponent& operator=(MeshComponent&&) = default;
    // eventually may need bool to track if VBO needs initializing
};

struct CameraComponent
{
    std::shared_ptr<Camera> camera;
};

// TODO: global getter/setter invoking ChunkComponent getter/setter
// TODO: get
struct ChunkMapComponent
{
private:
    entt::registry& m_Registry;
    std::unordered_map<int, std::unordered_map<int, entt::entity>> m_ChunkMap; // typedef ChunkMap?
public:
    // if performance suffers, consider O(1) alternative
//    std::map<std::pair<int, int>, entt::entity> m_ChunkMap;
    bool contains(const std::pair<int, int>& chunkLoc) const {
        return m_ChunkMap.contains(chunkLoc.first) && m_ChunkMap.at(chunkLoc.first).contains(chunkLoc.second);
    }
    void erase(const std::pair<int, int>& chunkLoc) {
        auto& innermostMap = m_ChunkMap.at(chunkLoc.first);
        innermostMap.erase(chunkLoc.second); // erase entry of chunk

        if (innermostMap.empty()) // erase outermost map if inner has no more entries
            m_ChunkMap.erase(chunkLoc.first);
    }
    entt::entity& operator[](const std::pair<int, int>& chunkLoc)
    {
        return m_ChunkMap[chunkLoc.first][chunkLoc.second];
    }
    entt::entity& operator[](std::pair<int, int>&& chunkLoc)
    {
        return m_ChunkMap[chunkLoc.first][chunkLoc.second];
    }
    ChunkMapComponent(entt::registry& registry)
        : m_Registry(registry) {};
    // copy constructor
    ChunkMapComponent(const ChunkMapComponent& other)
        : m_Registry(other.m_Registry), m_ChunkMap(other.m_ChunkMap) {};
    // should make this move assignable & constructable for entt
    ChunkMapComponent(ChunkMapComponent&& other)
        : m_Registry(other.m_Registry), m_ChunkMap(std::move(other.m_ChunkMap))
    {};
    ChunkMapComponent& operator=(ChunkMapComponent&& other) {
        if (this == &other)
            return *this;
        this->m_ChunkMap = std::move(other.m_ChunkMap);
        this->m_Registry = std::move(other.m_Registry);
        return *this;
    };
    ChunkMapComponent() = delete;

    // useful getters/util, no business logic
    // chunkOf(pos) accepts vec3 outputs x,z of parent chunk
    bool isLoaded(const std::pair<int, int>& chunkLoc) const
    {
        return this->contains(chunkLoc);
    }
    static std::pair<int, int> chunkOf(const glm::vec3& pos)
    {
        int xChunk = static_cast<int>(pos.x) / CHUNK_WIDTH * CHUNK_WIDTH;
        int zChunk = static_cast<int>(pos.z) / CHUNK_WIDTH * CHUNK_WIDTH;
        // above statement maps to upper right corner (instead of bottom left) for negative values
        if (pos.x < 0) xChunk -= CHUNK_WIDTH;
        if (pos.z < 0) zChunk -= CHUNK_WIDTH;

        return std::make_pair(xChunk, zChunk);
    }
    static std::pair<int, int> chunkOf(const glm::ivec3& pos)
    {
        int xChunk = pos.x / CHUNK_WIDTH * CHUNK_WIDTH;
        int zChunk = pos.z / CHUNK_WIDTH * CHUNK_WIDTH;
        // above statement maps to upper right corner (instead of bottom left) for negative values
        if (pos.x < 0) xChunk -= CHUNK_WIDTH;
        if (pos.z < 0) zChunk -= CHUNK_WIDTH;

        return std::make_pair(xChunk, zChunk);
    }
    // blockAt(pos) accepts vec3 outputs pointer to
    const Block* blockAt(glm::vec3 pos)
    {
        auto chunkLoc = chunkOf(pos);
        // account for indexing within chunk (0,0,0) to (WIDTH-1, HEIGHT-1, WIDTH-1)
        glm::ivec3 integralPosInChunk (static_cast<int>(pos.x) - chunkLoc.first,
                                static_cast<int>(pos.y),
                                static_cast<int>(pos.z) - chunkLoc.second);

        if (!isLoaded(chunkLoc)) // chunk does not exist
            throw std::runtime_error("[Runtime Exception] ChunkMapComponent::blockAt input parameter pos refers to unloaded chunk.");

        ChunkMapComponent& self = *this;
        ChunkComponent& chunkComp = m_Registry.get<ChunkComponent>(self[chunkLoc]);
        return chunkComp.blockAt(integralPosInChunk.x, integralPosInChunk.y, integralPosInChunk.z);
    }
    void setBlock(const glm::ivec3& blockPos, const BlockType type)
    {
        ChunkMapComponent& self = *this;
        auto chunkLoc = chunkOf(blockPos);
        ChunkComponent& chunkComp = m_Registry.get<ChunkComponent>(self[chunkLoc]);
        chunkComp.setBlock(blockPos, type);
    }

    void deleteChunk(const std::pair<int, int>& chunkLoc)
    {
        if (not this->contains(chunkLoc))
            throw std::runtime_error("[Runtime Exception] ChunkMapComponent::deleteChunk input parameter chunkLoc refers to unloaded chunk.");

        this->erase(chunkLoc); // remove from lookup search tree
        updateNeighbors(entt::null, chunkLoc); // update list of neighbors for surrounding chunk entities
    }

    void insertChunk(const entt::entity& e_Chunk, const std::pair<int, int>& chunkLoc)
    {
        ChunkMapComponent& self = *this;
        self[chunkLoc] = e_Chunk;
        updateNeighbors(e_Chunk, chunkLoc); // update list of adjacent neighbors
    }

    void updateNeighbors(const entt::entity& e_Chunk, const std::pair<int, int>& chunkLoc)
    {
        // accepts chunk being created or destroyed and its position
        // updates surrounding chunks' neighbors array to reflect creation/destruction

        // x defines WEST-EAST axis, z defines NORTH-SOUTH axis
        // goes EAST --> update WEST, goes SOUTH --> update NORTH
        // [CW, 0] [0, -CW] [-CW, 0] [0, CW]
        const static std::vector<int> dirDist {CHUNK_WIDTH, 0, -CHUNK_WIDTH, 0, CHUNK_WIDTH};
        const static std::vector<Direction> oppositeDirIdx {WEST, NORTH, EAST, SOUTH};
        const static std::vector<Direction> dirIdx {EAST, SOUTH, WEST, NORTH};

        // iterate over surrounding 4 chunks
        for (int i = 0; i < dirIdx.size(); i++)
        {
            // identify x,z coordinates of neighbor chunk
            std::pair<int, int> neighborPos(chunkLoc.first + dirDist[i], chunkLoc.second + dirDist[i + 1]);
            Direction dirTowardUpdatedChunk = oppositeDirIdx[i]; // e_Chunk is in this direction
            Direction dirTowardNeighbor = dirIdx[i];

            if (not this->contains(neighborPos)) // skip iteration if no neighbor exists in memory
                continue;

            // update neighbor array corresponding to neighboring chunk entity
            ChunkMapComponent& self = *this;
            entt::entity e_AdjChunk = self[neighborPos];
            std::vector<entt::entity>& adjChunkNeighbors = m_Registry.get<ChunkComponent>(e_AdjChunk).neighborEntities;
            adjChunkNeighbors[dirTowardUpdatedChunk] = e_Chunk;

            // update neighbor array of current chunk entity
            if (e_Chunk != entt::null)
                m_Registry.get<ChunkComponent>(e_Chunk).neighborEntities[dirTowardNeighbor] = e_AdjChunk;
        }
    }

};