
#pragma once

glm::vec3 getPlayerPos(const entt::registry& registry);

glm::vec3 getPlayerCameraDir(const entt::registry& registry);

std::tuple<const bool, const glm::ivec3> selectPlayerBlock(entt::registry& registry);

int sgn(float x);
