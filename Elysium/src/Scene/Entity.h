#pragma once

#include "Scene.h"
#include "Components.h"
#include "ECS.h"

class Entity
{
public:
	Entity() = default;
	Entity(ECS::Entity handle, Scene* scene);
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& addComponent(Args&&... args)
	{
		// assert !hascomponent
		T& component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		return component;
	}

	template<typename T>
	T& addComponent(const T& component)
	{
		// assert !hascomponent
		return m_Scene->m_Registry.emplaceCopy<T>(m_EntityHandle, component);
	}

	template<typename T>
	T& getComponent()
	{
		// assert hascomponent
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}

	template<typename T>
	bool hasComponent()
	{
		return m_Scene->m_Registry.has<T>(m_EntityHandle);
	}

	template<typename T>
	void removeComponent()
	{
		// assert HasComponent...
		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}

	operator bool() const { return m_EntityHandle != ECS::NullEntity; }
	operator ECS::Entity() const { return m_EntityHandle; }
	//operator uint32_t() const { return (uint32_t)m_EntityHandle; }

	Elysium::UUID GetUUID() { return getComponent<CId>().id;  }
	const std::string& GetName() { return getComponent<CTag>().tag; }

	bool operator== (const Entity& other) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	bool operator != (const Entity& other) const
	{
		return !(*this == other);
	}

private:
	ECS::Entity m_EntityHandle{ ECS::NullEntity };
	Scene* m_Scene = nullptr;
};
