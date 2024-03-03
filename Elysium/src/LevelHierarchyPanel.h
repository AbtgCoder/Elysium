#pragma once

#include "Level/Level.h"

class LevelHierarchyPanel
{
public:
	LevelHierarchyPanel() = default;
	LevelHierarchyPanel(const std::shared_ptr<Level>& level);

	void SetLevel(const std::shared_ptr<Level>& level);

	void OnImGuiRender();

	std::shared_ptr<Entity> GetInspectedEntity() const { return m_InspectedEntity; }
	void SetInspectedEntity(std::shared_ptr<Entity> entity);
private:
	template<typename T, typename... TArgs>
	void DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs);

	void DrawEntityNode(std::shared_ptr<Entity> entity);
private:
	std::shared_ptr<Level> m_Level;
	std::shared_ptr<Entity> m_InspectedEntity;
};