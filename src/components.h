#pragma once

#include <string>
#include <glm/vec3.hpp>

struct TagComponent
{
    std::string tag;
};

struct PositionComponent
{
    // eventually change to array representation with [xPos1 yPos1 zPos1 t1Coord1 t2Coord1...]
    glm::vec3 pos;
};

struct InputComponent
{
    // i think glfw uses some integer enum for keys
    int key;
};