#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

#include <entt.hpp>
#include "entity.h"


int main() {
    World world;

//    Chunk::createChunk(world);

    // ECS game loop
    while (!world.isDestroyed())
    {
        world.update();
    }

    return 0;
}