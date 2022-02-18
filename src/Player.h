
#pragma once

std::pair<int, int> chunkOf(const glm::vec3& pos);

glm::vec3 getPlayerPos(const entt::registry& registry);

const Block* selectPlayerBlock(const entt::registry& registry);
