#pragma once

class ScriptableEntity
{
public:
	virtual ~ScriptableEntity() {}

	template<typename T>
	T& getComponent()
	{
		//return m_Entity.getComponent<T>();
		//return m_EntityManager->getComponent<T>(m_Entity);
	}
protected:
	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(float ts) {}
//private:
	//Entity m_Entity;
	//EntityManager* m_EntityManager = nullptr;

	friend class Scene;
};