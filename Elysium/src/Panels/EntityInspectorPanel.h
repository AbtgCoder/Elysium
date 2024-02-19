#pragma once

#include "../Entity.h"

class EntityInspectorPanel
{
public:
	EntityInspectorPanel()=default;

	void setInspectedEntity(std::shared_ptr<Entity>& entity);
	std::shared_ptr<Entity> getInspectedEntity() const;

	void OnImGuiRender();
private:
	template<typename T>
	void DisplayAddComponentEntry(const std::string& entryName);
private:
	std::shared_ptr<Entity> m_inspectedEntity;
};