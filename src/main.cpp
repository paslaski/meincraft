#include <iostream>
#include <fstream>
#include <sstream>

#include "world.h"

int main() {
    World world;

    // ECS game loop
    while (!world.isDestroyed())
    {
        world.update();
    }

    return 0;
}