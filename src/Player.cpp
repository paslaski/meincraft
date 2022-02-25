
#include <cmath>
#include "entt/entt.hpp"
#include "Components.h"
#include "Player.h"
#include "BlockPool.h"

glm::vec3 getPlayerPos(const entt::registry& registry)
{
    auto playerView = registry.view<CameraComponent>();
    for (auto& e_Player : playerView)
        return registry.get<CameraComponent>(e_Player).camera->Position;
}

glm::vec3 getPlayerCameraDir(const entt::registry& registry)
{
    auto playerView = registry.view<CameraComponent>();
    for (auto& e_Player : playerView)
        return registry.get<CameraComponent>(e_Player).camera->Front;
}

std::tuple<const bool, const glm::ivec3> selectPlayerBlock(entt::registry& registry)
{
    // initialization phase
    glm::vec3 origin = getPlayerPos(registry);
    glm::vec3 dir = getPlayerCameraDir(registry);
    glm::ivec3 step (sgn(dir.x), sgn(dir.y), sgn(dir.z));
    // ray equation: u→ + tv→ for t ≥ 0
    // find t value for ray to cross first voxel barrier (for each dir)
    /*
     * if (stepX == 0) t.x = 0
     * uNext.x = if (stepX == 1) floor(u.x) else ceil(u.x) ?
     * u.x + t*v.x = uNext.x
     * t.x = (uNext.x - u.x)/v.x
     */
    glm::vec3 tMax ((step.x == 0) ? std::numeric_limits<float>::infinity() :
                    (step.x == 1) ? (ceil(origin.x) - origin.x)/dir.x : (origin.x - floor(origin.x))/dir.x,
                    (step.y == 0) ? std::numeric_limits<float>::infinity() :
                    (step.y == 1) ? (ceil(origin.y) - origin.y)/dir.y : (origin.y - floor(origin.y))/dir.y,
                    (step.z == 0) ? std::numeric_limits<float>::infinity() :
                    (step.z == 1) ? (ceil(origin.z) - origin.z)/dir.z : (origin.z - floor(origin.z))/dir.z);
    // find t value for ray to cross any full 1-length voxel (for each dir)
    /*
     * t*v.x = stepX
     * t(v.x) = stepX
     * t = stepX/(v.x)
     */
    glm::vec3 tDelta ((step.x != 0) ? static_cast<float>(step.x)/dir.x : std::numeric_limits<float>::infinity(),
                      (step.y != 0) ? static_cast<float>(step.y)/dir.y : std::numeric_limits<float>::infinity(),
                      (step.z != 0) ? static_cast<float>(step.z)/dir.z : std::numeric_limits<float>::infinity());

    // incremental traversal phase
    // retrieving chunk map to query & modify chunk states
    auto chunkMapView = registry.view<ChunkMapComponent>();
    entt::entity e_ChunkMap = chunkMapView.front();
    ChunkMapComponent& chunkMap = registry.get<ChunkMapComponent>(e_ChunkMap);
    // retrieve blockpool
    const auto& blockPool = BlockPool::getPoolInstance();

    // TODO: eventually change to first target outside of origin? unsure what is desired mechanic
    // TODO: player range
    // manhattan vs euc
    std::pair<int, int> curChunk = chunkMap.chunkOf(origin);
    if (!chunkMap.isLoaded(curChunk))
        return {false, {-1, -1, -1}};
    const Block* curBlock = chunkMap.blockAt(origin);

    while (curBlock == blockPool.getBlockPtr(AIR))
    {
        if (tMax.x <= tMax.y && tMax.x <= tMax.z)
        {
            origin.x += step.x;
            tMax.x += tDelta.x;
        } else if (tMax.y <= tMax.x && tMax.y <= tMax.z)
        {
            origin.y += step.y;
            tMax.y += tDelta.y;
        } else
        { // tMax.z is minimum
            origin.z += step.z;
            tMax.z += tDelta.z;
            if (origin.z < 0 || origin.z >= CHUNK_HEIGHT)
                return {false, {-1, -1, -1}};
        }

        if (chunkMap.chunkOf(origin) != curChunk) {
            curChunk = chunkMap.chunkOf(origin);
            if (!chunkMap.isLoaded(curChunk))
                return {false, {-1, -1, -1}};
        }
        curBlock = chunkMap.blockAt(origin);
    }

    return {true, origin};
}

int sgn(float x)
{
    if (x > 0.0f)
        return 1;
    if (x < 0.0f)
        return -1;
    return 0;
}