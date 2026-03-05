#pragma once

#include "Animation/AnimationController.h"
#include "Scene/Scene.h"
#include "Scene/Entity.h"

#include "../Helper/ImGuiHelper.h"

class AnimationPanel
{
public:
	AnimationPanel() = default;

	void OnImGuiRender();

	void Open() { m_Open = true; }
	void Close() { m_Open = false; }

	Entity GetInspectedEntity() const { return m_InspectedEntity; }
	void SetInspectedEntity(Entity entity);
private:
	void CreateAnimationClip();
private:
	bool m_Open = true; //TODO: make an actual panel manager for managing all this...
	
	Entity m_InspectedEntity;

	int m_SelectedStateIndex = -1;
	int m_SelectedTransitionIndex = -1;

	char m_NewParameterName[128] = "";
	int m_NewParameterType = 0;

	char m_NewTransitionToState[128] = "";
};