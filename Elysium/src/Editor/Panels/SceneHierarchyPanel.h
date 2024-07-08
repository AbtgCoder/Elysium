#pragma once

#include "Scene/Scene.h"

class SceneHierarchyPanel
{
public:
	SceneHierarchyPanel() = default;
	SceneHierarchyPanel(const std::shared_ptr<Scene>& Scene);

	void SetScene(const std::shared_ptr<Scene>& Scene);

	void OnImGuiRender();

	Entity GetInspectedEntity() const { return m_InspectedEntity; }
	void SetInspectedEntity(Entity entity);
private:
	template<typename T, typename... TArgs>
	void DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs);
	template<typename T, typename UIFunction>
	void DrawComponentGUI(const std::string& name, Entity entity, UIFunction uiFunction);
	void DrawEntityNode(Entity entity);
private:
	std::shared_ptr<Scene> m_Scene;
	Entity m_InspectedEntity;

	bool m_MouseButtonPressed = false;
};