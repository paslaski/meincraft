
#include "World.h"

World::World()
// must call createPlayer() before user camera can be passed to renderSystem & inputSystem
    : renderSystem((createPlayer(), retrievePlayerCamera())), registry(entt::registry()),
      inputSystem(renderSystem.get_window(), renderSystem.get_camera()),
      chunkLoaderSystem(registry),
      chunkMeshingSystem(ChunkMeshingSystem())
{
    inputSystem.assign_window_callbacks();

    // create 5x5 chunk grid for testing
//    for (int x = 0; x <= 16*10; x+=16)
//        for (int z = 0; z <= 16*10; z+=16)
//            chunkLoaderSystem.chunkGenerator.generateChunk(glm::vec3{x, 0, z});
}

World::~World()
{}

void World::update()
{
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // TODO: optimize order (e.g. delete unnecessary chunks before rendering)
    inputSystem.update(registry, deltaTime);
    chunkLoaderSystem.update(registry);
    chunkMeshingSystem.update(registry);
    renderSystem.update(registry);
}

bool World::isDestroyed()
{
    return glfwWindowShouldClose(renderSystem.get_window());
}

void World::createPlayer() {
    entt::entity player = registry.create();
    registry.emplace<CameraComponent>(player, std::make_shared<Camera>(glm::vec3(0.0f, 100.0f, 0.0f)));
    // eventually add inventory component
}

std::shared_ptr<Camera> World::retrievePlayerCamera()
{
    // there should only be one player with camera per game, so return first value found
    auto playerView = registry.view<CameraComponent>();
    for (auto e_Player : playerView)
        return registry.get<CameraComponent>(e_Player).camera;
}
