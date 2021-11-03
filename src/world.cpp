#include "world.h"

#include <string>
#include "entity.h"


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

Entity World::create_entity(std::string tag)
{
    Entity entity(registry.create(), this);
    entity.add_component<TagComponent>(tag);
    return entity;
}

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
