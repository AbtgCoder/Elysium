#pragma once

#include "Entity.h"


class EntityManager
{
public:

	void update();

	Entity addEntity();
	const std::vector<Entity>& GetEntities();
private:
	void removeDeadEntities(std::vector<Entity>& entityVec);

	std::vector<Entity> m_Entities;
	std::vector<Entity> m_EntitiesToAdd;
};