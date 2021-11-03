#pragma once

#include <entt.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "components.h"
#include "rendersystem.h"
#include "inputsystem.h"
#include "ChunkLoaderSystem.h"
#include "ChunkMeshingSystem.h"

class Entity;

class World
{
private:
    entt::registry registry;

    RenderSystem renderSystem;
    InputSystem inputSystem;
    ChunkLoaderSystem chunkLoaderSystem;
    ChunkMeshingSystem chunkMeshingSystem;

    // timing
    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;

    GLFWwindow* window;

public:
    World();
    ~World();

    void update();

    bool isDestroyed();

    friend class Entity;
};