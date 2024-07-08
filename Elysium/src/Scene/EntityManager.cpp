#include "EntityManager.h"


void EntityManager::update()
{
	// adding entities
	for (auto e : m_EntitiesToAdd)
	{
		m_Entities.push_back(e);
	}

	// deleting entities
	removeDeadEntities(m_Entities);

	m_EntitiesToAdd.clear();
}

Entity EntityManager::addEntity()
{
	Entity e = Entity(EntityMemoryPool::Instance().addEntity());
	m_EntitiesToAdd.push_back(e);
	return e;
}

const std::vector<Entity>& EntityManager::GetEntities()
{
	return m_Entities;
}

void EntityManager::removeDeadEntities(std::vector<Entity>& entityVec)
{
//	std::vector<OptEntity> entitiesToRemove;
//	for (auto& e : entityVec)
//	{
//		if (!e.isActive())
//		{
//			entitiesToRemove.push_back(e);
//		}
//	}
//
//	for (auto& e : entitiesToRemove)
//	{
//		// find index of entity
//		auto it = std::find(entityVec.begin(), entityVec.end(), e);
//		// if index found
//		if (it != entityVec.end())
//		{
//			entityVec.erase(it);
//		}
//
//	}

	entityVec.erase(std::remove_if(entityVec.begin(), entityVec.end(), [](Entity& entity) {
		return !entity.isActive();
		}), entityVec.end());
}
