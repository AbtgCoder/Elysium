#pragma once

#include "Entity.h"


class EntityManager
{
public:

	void update();

	Entity addEntity();
	const std::vector<Entity>& GetEntities();

    template<typename T>
    bool hasComponent(Entity e) const
    {
        return EntityMemoryPool::Instance().hasComponent<T>(e.id());
    }

    template<typename T>
    T& getComponent(Entity e)
    {
        return EntityMemoryPool::Instance().getComponent<T>(e.id());
    }

    template<typename T, typename... Args>
    T& addComponent(Entity e, Args&&... args)
    {
        return EntityMemoryPool::Instance().addComponent<T>(e.id(), std::forward<Args>(args)...);
    }

    template<typename T>
    void removeComponent(Entity e)
    {
        EntityMemoryPool::Instance().removeComponent<T>(e.id());
    }
private:
	void removeDeadEntities(std::vector<Entity>& entityVec);

	std::vector<Entity> m_Entities;
	std::vector<Entity> m_EntitiesToAdd;
};