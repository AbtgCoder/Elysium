#pragma once

//#include <string>
//#include <vector>

#include "EntityMemoryPool.h"

class Entity
{
public:
	Entity() = default;

	Entity(size_t index)
		: m_Id(index), m_IsValidEntity(true) {}

	size_t id() const { return m_Id; }

	bool isActive()
	{
		return EntityMemoryPool::Instance().isActive(m_Id);
	}

	template<typename T>
	T& getComponent()
	{
		return EntityMemoryPool::Instance().getComponent<T>(m_Id);
	}

	// has component
	template<typename T>
	bool hasComponent()
	{
		return EntityMemoryPool::Instance().hasComponent<T>(m_Id);
	}
	// add component
	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs)
	{
		return EntityMemoryPool::Instance().addComponent<T>(m_Id, std::forward<TArgs>(mArgs)...);
	}
	// remove component
	template <typename T>
	void removeComponent()
	{
		EntityMemoryPool::Instance().removeComponent<T>(m_Id);
	}

	void destroy()
	{
		EntityMemoryPool::Instance().destroy(m_Id);
	}

	operator bool() const { return m_IsValidEntity; }

	Elysium::UUID GetUUID() { return getComponent<CId>().id; }

private:
	bool m_IsValidEntity = false;
	size_t m_Id;
};
