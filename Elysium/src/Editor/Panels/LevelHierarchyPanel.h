#pragma once

#include "Level/Level.h"

class LevelHierarchyPanel
{
public:
	LevelHierarchyPanel() = default;
	LevelHierarchyPanel(const std::shared_ptr<Level>& level);

	void SetLevel(const std::shared_ptr<Level>& level);

	void OnImGuiRender();

	Entity GetInspectedEntity() const { return m_InspectedEntity; }
	void SetInspectedEntity(Entity entity);
private:
	template<typename T, typename... TArgs>
	void DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs);

	void DrawEntityNode(Entity entity);
private:
	std::shared_ptr<Level> m_Level;
	Entity m_InspectedEntity;
};