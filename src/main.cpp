#include <iostream>
#include <fstream>
#include <sstream>

#include <math.h>

#include <entt.hpp>
#include "entity.h"

void createEntity(World& world, glm::vec3 pos);

int main() {
    World world;

    // world space positions of our cubes
    std::vector<glm::vec3> cubePositions = {
            glm::vec3( 0.0f,  0.0f,  0.0f),
            glm::vec3(0.0f, 1.0f, 1.0f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3( 1.3f, -2.0f, -2.5f),
            glm::vec3( 1.5f,  2.0f, -2.5f),
            glm::vec3( 1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // create dirt blocks for testing purposes
    for (int i = 0; i < cubePositions.size(); i++)
    {
        createEntity(world, cubePositions[i]);
    }

    // ECS game loop
    while (!world.isDestroyed())
    {
        world.update();
    }

    return 0;
}

void createEntity(World& world, glm::vec3 pos)
{
    Entity e_block = world.create_entity("dirt_block");
    e_block.add_component<PositionComponent>(pos);
}