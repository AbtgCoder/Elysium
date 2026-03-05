#pragma once

#include "AnimationClip.h"

enum class AnimatorParameterType
{
	Bool,
	Float,
	Trigger
};

struct AnimatorParameter
{
	std::string Name;
	AnimatorParameterType Type;

	bool BoolValue = false;
	float FloatValue = 0.0f;

	bool Triggered = false; // for triggers
};

enum class AnimatorConditionType
{
	BoolTrue,
	BoolFalse,
	FloatGreater,
	FloatLess,
	Trigger
};

struct AnimatorCondition
{
	std::string ParameterName = "";
	AnimatorConditionType ConditionType;
	float Threshold = 0.0f; // for float comparisons
};

struct AnimationTransition
{
	std::string FromState;
	std::string ToState;

	std::vector<AnimatorCondition> Conditions;

	bool HasExitTime = false;
	float ExitTime = 1.0f; // normalized 0-1
};

struct AnimationState
{
	std::string Name;
	std::shared_ptr<AnimationClip> Clip = nullptr;

	std::vector<AnimationTransition> Transitions;
};

class AnimationController
{
public:
	AnimationController() = default;

	void AddState(const std::string& name, std::shared_ptr<AnimationClip> clip);
	void AddTransition(const AnimationTransition& transition);
	void AddParameter(const std::string& name, AnimatorParameterType type);

	void SetBool(const std::string& name, bool value);
	void SetFloat(const std::string& name, float value);
	void SetTrigger(const std::string& name);


	void Play(const std::string& stateName);

	void Update(float dt);

	AnimationFrame GetCurrentFrame() const;

private:
	bool EvaluateTransitions();
private:
	std::unordered_map<std::string, AnimationState> m_States;
	std::unordered_map<std::string, AnimatorParameter> m_Parameters;

	std::string m_CurrentState = "";
	float m_StateTimer = 0.0f;
	size_t m_CurrentFrame = 0;

	friend class Scene;
	friend class AnimationPanel;
	friend class SceneSerializer;
};