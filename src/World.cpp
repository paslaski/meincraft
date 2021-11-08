
#include "World.h"

World::World()
    : renderSystem(RenderSystem()), registry(entt::registry()),
      inputSystem(renderSystem.get_window(), renderSystem.get_camera()),
      chunkLoaderSystem(registry),
      chunkMeshingSystem(ChunkMeshingSystem())
{
    inputSystem.assign_window_callbacks();
//    chunkLoaderSystem.createChunk(registry);
//    chunkLoaderSystem = ChunkLoaderSystem(registry);
    for (int x = 0; x <= 16*5; x+=16)
        for (int z = 0; z <= 16*5; z+=16)
            chunkLoaderSystem.chunkGenerator.generateChunk(glm::vec3{x, 0, z});
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
