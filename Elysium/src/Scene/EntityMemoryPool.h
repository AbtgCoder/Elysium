#pragma once
#include "Components.h"

#include <tuple>
#include <vector>

typedef std::tuple <
	std::vector<CId>, 
	std::vector<CParent>,
	std::vector<CTag>,
	std::vector<CTransform>,
	std::vector<CLifespan>,
	std::vector<CInput>,
	std::vector<CRigidBody>,
	std::vector<CBoundingBox>,
	std::vector<CPolygonCollider>,
	std::vector<CState>,
	std::vector<CScore>,
	std::vector<CSpriteRenderer>,
	std::vector<CRectangle>,
	std::vector<CCircle>,
	std::vector<CPolygon>,
	std::vector<CCircleCollider>,
	std::vector<CPhysicsMaterial>,
	std::vector<CJoint>,
	std::vector<CNativeScriptComponent>,
	std::vector<CCamera>
> EntityComponentVectorTuple;

static size_t MAX_ENTITIES = 100000;

class EntityMemoryPool
{
public:
	static EntityMemoryPool& Instance()
	{
		static EntityMemoryPool pool(MAX_ENTITIES);
		return pool;
	}

	bool isActive(size_t entityId)
	{
		return m_active[entityId];
	}

	size_t addEntity()
	{
		// get nearest available index
		size_t index = 0; 
		for (size_t i = 0; i < m_active.size(); i++)
		{
			if (!m_active[i])
			{
				if (index != i)
				{
					index = i;
					// TODO: set all component values to zero
					removeComponent<CId>(index);
					removeComponent<CTag>(index);
				}
				break;
			}
		}
		

		m_active[index] = true;
		return index;
		//return OptEntity(index);
	}

	void destroy(size_t entityId)
	{
		m_active[entityId] = false;
	}

	template <typename T>
	T& getComponent(size_t entityID)
	{
		return std::get<std::vector<T>>(m_pool)[entityID];
	}

	template <typename T>
	bool hasComponent(size_t entityID)
	{
		return getComponent<T>(entityID).has;
	}

	template <typename T, typename... TArgs>
	T& addComponent(size_t entityID, TArgs&&... mArgs)
	{
		auto& component = getComponent<T>(entityID);
		component = T(std::forward<TArgs>(mArgs)...);
		component.has = true;
		return component;
	}

	template <typename T>
	void removeComponent(size_t entityID)
	{
		getComponent<T>(entityID) = T();
	}

private:
	size_t m_numEntities;
	EntityComponentVectorTuple m_pool;
	std::vector<bool> m_active;
	EntityMemoryPool(size_t maxEntities)
		: m_numEntities(0), m_pool(maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities, maxEntities), m_active(maxEntities, false) {}
};