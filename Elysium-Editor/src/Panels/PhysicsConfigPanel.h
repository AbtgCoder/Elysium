#pragma once

#include "Scene/Scene.h"

class PhysicsConfigPanel
{
public:
	PhysicsConfigPanel() = default;
	PhysicsConfigPanel(const std::shared_ptr<Scene>& Scene);

	void SetScene(const std::shared_ptr<Scene>& Scene);

	void OnImGuiRender();
private:
	std::shared_ptr<Scene> m_Scene;
};
