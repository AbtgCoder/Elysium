#pragma once

#include "Entity.h"
#include <vector>
#include <map>
#include <memory>

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager
{
public:
	EntityManager();

	void update();

	std::shared_ptr<Entity> addEntity(const std::string& tag);
	std::shared_ptr<Entity> addEntity(const std::shared_ptr<Entity>& originalEntity);
	const EntityVec& getEntities(); 
	const EntityVec& getEntities(const std::string& tag);
	const EntityMap& getEntityMap();
private:
	EntityVec m_entities;
	EntityVec m_entitiesToAdd;
	EntityMap m_entityMap;
	size_t m_totalEntities = 0;

	void removeDeadEntities(EntityVec& vec);
	
	
	template <typename... Components>
	void duplicateComponents(const std::shared_ptr<Entity>& originalEntity, const std::shared_ptr<Entity>& newEntity)
	{
		for_each(originalEntity->m_components, originalEntity, newEntity);
	}

	
	template <std::size_t I = 0, typename... Tp>
	inline typename std::enable_if < I == sizeof...(Tp), void>::type
		for_each(std::tuple<Tp...>&, const std::shared_ptr<Entity>& originalEntity, const std::shared_ptr<Entity>& newEntity) {}

	template <std::size_t I=0, typename... Tp>
	inline typename std::enable_if<I < sizeof...(Tp), void>::type
		for_each(std::tuple<Tp...>& t, const std::shared_ptr<Entity>& originalEntity, const std::shared_ptr<Entity>& newEntity)
	{
		//std::cout << std::get<I>(t).has << "\n";
		newEntity->addComponent(std::get<I>(t));
		for_each<I + 1, Tp...>(t, originalEntity, newEntity);
	}


};