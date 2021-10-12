#include "entity.h"

Entity::Entity(entt::entity handle, World* world)
        : m_handle(handle), m_world(world)
{}