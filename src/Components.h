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

    // include array of 8 bit enums corresponding to block types
    std::vector<BlockType> blocks; // = std::vector<BlockType>(CHUNK_WIDTH * CHUNK_HEIGHT * CHUNK_WIDTH, AIR);
    // classifies biome for each x,z index
    std::vector<BiomeType> biomeMap; // = std::vector<BiomeType>(CHUNK_WIDTH * CHUNK_WIDTH, GrassBiome);

    BlockType blockAt(int x, int y, int z) {
        return blocks[x + (z * CHUNK_WIDTH) + (y * CHUNK_WIDTH * CHUNK_WIDTH)];
    }
};

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
    ~MeshComponent() { glDeleteBuffers(1, &blockVBO); }
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
