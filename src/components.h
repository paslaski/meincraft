#pragma once

#include <string>
#include <glm/vec3.hpp>
#include "block.h"
#include "chunk.h"

struct TagComponent
{
    std::string tag;
};

struct PositionComponent
{
    // eventually change to array representation with [xPos1 yPos1 zPos1 t1Coord1 t2Coord1...]
    glm::vec3 pos;
};

struct DrawableComponent
{
    bool hasUpdated;
    float vbo[];
};

struct BlockComponent
{
    // include array of 8 bit enums corresponding to block types
    std::vector<BlockType> blocks = std::vector<BlockType>(CHUNK_SIZE*CHUNK_SIZE*CHUNK_SIZE, AIR);

    // retrieve block at 3D index (i, j, k) from 1D array
    BlockType at(int i, int j, int k) {
        return blocks[i + (j * CHUNK_SIZE) + (k * CHUNK_SIZE*CHUNK_SIZE)];
    }
};

struct InputComponent
{
    // i think glfw uses some integer enum for keys
    int key;
};