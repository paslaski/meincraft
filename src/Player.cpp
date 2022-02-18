
#include "entt/entt.hpp"
#include "Components.h"
#include "Player.h"

std::pair<int, int> chunkOf(const glm::vec3& pos)
{
    int xChunk = (int) pos.x / CHUNK_WIDTH * CHUNK_WIDTH;
    int zChunk = (int) pos.z / CHUNK_WIDTH * CHUNK_WIDTH;
    // above statement maps to upper right corner (instead of bottom left) for negative values
    if (pos.x < 0) xChunk -= CHUNK_WIDTH;
    if (pos.z < 0) zChunk -= CHUNK_WIDTH;

    return std::make_pair(xChunk, zChunk);
}

glm::vec3 getPlayerPos(const entt::registry& registry)
{
    auto playerView = registry.view<CameraComponent>();
    for (auto& e_Player : playerView)
        return registry.get<CameraComponent>(e_Player).camera->Position;
}

const Block* selectPlayerBlock(const entt::registry& registry)
{

    return nullptr;
}
