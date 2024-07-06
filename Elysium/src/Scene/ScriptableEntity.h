#pragma once

#include "Entity.h"

class ScriptableEntity
{
public:
	virtual ~ScriptableEntity() {}

	template<typename T>
	T& getComponent()
	{
		return m_Entity.getComponent<T>();
	}
protected:
	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(float ts) {}
private:
	Entity m_Entity;
	friend class Scene;
};