#pragma once

#include <entt.hpp>

class World;

const int CHUNK_SIZE = 16;

class Chunk {
public:
    static void createChunk(World& world);

};


