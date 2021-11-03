
#include "world.h"

World::World()
    : renderSystem(RenderSystem()), registry(entt::registry()),
      inputSystem(renderSystem.get_window(), renderSystem.get_camera()),
      chunkLoaderSystem(ChunkLoaderSystem()), chunkMeshingSystem(ChunkMeshingSystem())
{
    inputSystem.assign_window_callbacks();
    chunkLoaderSystem.createChunk(registry);
}

World::~World()
{}

void World::update()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    inputSystem.update(registry, deltaTime);
    chunkLoaderSystem.update(registry);
    chunkMeshingSystem.update(registry);
    renderSystem.update(registry);
}

bool World::isDestroyed()
{
    return glfwWindowShouldClose(renderSystem.get_window());
}
