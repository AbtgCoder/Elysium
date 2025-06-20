#pragma once

#include "AnimationClip.h"

struct AnimationState
{
	std::string Name;
	std::shared_ptr<AnimationClip> Clip = nullptr;
	//TODO: transition info ?? or store that separetly in "struct AnimationTransition"
};

class AnimationController
{
public:
	AnimationController() = default;

	void AddState(const std::string& name, std::shared_ptr<AnimationClip> clip);

	void Play(const std::string& stateName);

	void Update(float dt);

	AnimationFrame GetCurrentFrame() const;

private:
	std::unordered_map<std::string, AnimationState> m_States;
	std::string m_CurrentState = "";
	//TODO: std::string m_StartState = "";

	float m_StateTimer = 0.0f;
	size_t m_CurrentFrame = 0;

	friend class Scene;
	friend class AnimationPanel;
	friend class SceneSerializer;
};