#pragma once

#include <string>
#include <memory>
#include <glm/vec3.hpp>
#include "Block.h"
#include "Chunk.h"
#include "Texture.h"
#include "Biome.h"
#include "Camera.h"

struct PositionComponent
{
    // eventually change to array representation with [xPos1 yPos1 zPos1 t1Coord1 t2Coord1...]
    glm::vec3 pos;
};

struct ChunkComponent
{
    bool hasChanged; // useful to determine if new meshes should be generated

    // "points" to neighboring chunks, but not necessary to use pointers as entities are just integers
    // convention: indexed with [NORTH, SOUTH, WEST, EAST] per enum
    std::vector<entt::entity> neighborEntities {entt::null, entt::null, entt::null, entt::null};

    // include array of 8 bit enums corresponding to block types
    std::vector<BlockType> blocks; // = std::vector<BlockType>(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, AIR);
    // classifies biome for each x,z index
    std::vector<BiomeType> biomeMap; // = std::vector<BiomeType>(CHUNK_WIDTH * CHUNK_WIDTH, GrassBiome);

    BlockType blockAt(int x, int y, int z) {
        return blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)];
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
    // swap & pop means destructor called twice --> must call glDeleteBuffers outside
    MeshComponent(const MeshComponent&) = delete;
    MeshComponent operator=(const MeshComponent&) = delete;
    MeshComponent(MeshComponent&&) = default;
    MeshComponent& operator=(MeshComponent&&) = default;
    // eventually may need bool to track if VBO needs initializing
};

struct CameraComponent
{
    std::shared_ptr<Camera> camera;
};
