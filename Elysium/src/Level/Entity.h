#pragma once

#include "Components.h"
#include <memory>
#include <string>
#include <tuple>
#include <utility>

typedef std::tuple<
	CTag,
	CTransform,
	CLifespan,
	CInput,
	CBoundingBox,
	CPolygonCollider,
	CGravity,
	CState,
	CScore,
	CSpriteRenderer
> ComponentTuple;

class Entity
{
	friend class EntityManager;
public:

	
	// private member access functions
	bool isActive() const;
	const std::string& tag() const;
	const size_t id() const;
	void destroy();

	template<typename T>
	T& getComponent()
	{
		return std::get<T>(m_components); // look for component in the tuple
	}

	template<typename T>
	const T& getComponent() const
	{
		return std::get<T>(m_components);
	}

	template <typename T>
	bool hasComponent() const
	{
		return getComponent<T>().has;
	}

	template <typename T>
	T& addComponent(T& originalComponent)
	{
		auto& newComponent = getComponent<T>();
		newComponent = T(originalComponent);
		newComponent.has = true;
		return newComponent;
	}

	template <typename T, typename... TArgs>
	T& addComponent(TArgs&&... mArgs)
	{
		auto& component = getComponent<T>();
		component = T(std::forward<TArgs>(mArgs)...);
		component.has = true;
		return component;
	}

	template<typename T>
	void removeComponent()
	{
		getComponent<T>() = T();
	}

	
private:
	bool m_active = true;
	size_t m_id = 0;
	std::string m_tag = "default";
	ComponentTuple m_components;
	
	Entity(const size_t id, const std::string& tag);

};