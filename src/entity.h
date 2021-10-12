#pragma once

#include <entt.hpp>

#include "world.h"

class Entity
{
private:
    entt::entity m_handle = entt::null;
    World* m_world = NULL;

public:
    Entity(entt::entity handle, World* world);

    inline entt::entity get_handle() { return m_handle; }

    template<typename T, typename... Args>
    T& add_component(Args&&... args)
    {
        return m_world->registry.emplace<T>(m_handle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& get_component()
    {
        return m_world->registry.get<T>(m_handle);
    }

    template<typename T>
    void remove_component()
    {
        return m_world->registry.remove<T>(m_handle);
    }

//    template<typename T>
//    bool has_component()
//    {
//        return m_world->registry.has<T>(m_handle);
//    }

};