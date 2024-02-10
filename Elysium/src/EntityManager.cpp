#include "EntityManager.h"
#include <algorithm>

EntityManager::EntityManager()
{
}

void EntityManager::update()
{	
	// adding entities
	for (auto& e : m_entitiesToAdd)
	{
		m_entities.push_back(e);
		m_entityMap[e->tag()].push_back(e);
	}

	// deleting entities
	removeDeadEntities(m_entities);
	for (auto& [tag, entityVec] : m_entityMap)
	{
		removeDeadEntities(entityVec);
	}

	/*if (m_entitiesToAdd.size() > 0) // TODO: do this for optimization, but then it needs to be called whenever you change rendering layer via entity inspector
	{*/
		std::sort(m_entities.begin(), m_entities.end(), [](const std::shared_ptr<Entity>& a, const std::shared_ptr<Entity>& b) {
			return a->getComponent<CAnimation>().layer < b->getComponent<CAnimation>().layer;
			});
	//}

	// clearing m_entitiesToAdd 
	m_entitiesToAdd.clear();

}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string& tag)
{
	//auto e = std::make_shared<Entity>(m_totalEntities++, tag);
	auto e = std::shared_ptr<Entity>(new Entity(m_totalEntities++, tag));
	m_entitiesToAdd.push_back(e);
	return e;
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::shared_ptr<Entity>& originalEntity)
{
	auto newEntity = std::shared_ptr<Entity>(new Entity(m_totalEntities++, originalEntity->tag()));
	duplicateComponents(originalEntity, newEntity);
	m_entitiesToAdd.push_back(newEntity);
	return newEntity;
}

const EntityVec& EntityManager::getEntities()
{
	return m_entities;
}

const EntityVec& EntityManager::getEntities(const std::string& tag)
{
	return m_entityMap[tag];
}

const EntityMap& EntityManager::getEntityMap()
{
	return m_entityMap;
}

void EntityManager::removeDeadEntities(EntityVec& vec)
{
	EntityVec entitiesToRemove;
	for (auto& e : vec)
	{
		if (!e->isActive())
		{
			entitiesToRemove.push_back(e);
		}
	}
	
	for (auto& e : entitiesToRemove)
	{
		// find index of entity
		auto it = std::find(vec.begin(), vec.end(), e);
		// if index found
		if (it != vec.end())
		{
			vec.erase(it);
		}

	}
}
