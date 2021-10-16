#include "world.h"

#include <string>
#include "entity.h"


World::World() : renderSystem(RenderSystem()), inputSystem(renderSystem.get_window(), renderSystem.get_camera())
{
    inputSystem.assign_window_callbacks();
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
    renderSystem.update(registry);
}

bool World::isDestroyed()
{
    return glfwWindowShouldClose(renderSystem.get_window());
}
