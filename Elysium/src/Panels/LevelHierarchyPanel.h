#pragma once

#include "../EntityManager.h"
#include "../Entity.h"

class LevelHierarchyPanel
{
public :
	LevelHierarchyPanel() = default;
	LevelHierarchyPanel(EntityManager& em);

	void setLevelEM(EntityManager& em);

	void OnImGuiRender();

	std::shared_ptr<Entity> getSelectedEntity() const { return m_inspectedEntity; }
	void setSelectedEntity(std::shared_ptr<Entity> entity);
private:
	void drawEntityNode(std::shared_ptr<Entity> entity);
private:
	EntityManager m_levelEM;
	std::shared_ptr<Entity> m_inspectedEntity;
};