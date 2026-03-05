#include "AnimationController.h"

#include "core/Logger.h"

void AnimationController::AddState(const std::string& name, std::shared_ptr<AnimationClip> clip)
{
	m_States[name] = { name, clip };
}

void AnimationController::AddTransition(const AnimationTransition& transition)
{
	if (!m_States.contains(m_CurrentState))
	{
		Logger::Log("tried adding transition without a current state", "Animator", LOG_TYPE::WARNING);
		return;
	}

	auto& state = m_States[m_CurrentState];
	state.Transitions.push_back(transition);
}

void AnimationController::AddParameter(const std::string& name, AnimatorParameterType type)
{
	AnimatorParameter param;
	param.Name = name;
	param.Type = type;
	m_Parameters[name] = param;
}

void AnimationController::SetBool(const std::string& name, bool value)
{
	if (!m_Parameters.contains(name))
	{
		Logger::Log("parameter by this name doesn't exist", "Animator", LOG_TYPE::WARNING);
		return;
	}
	auto& param = m_Parameters.at(name);
	param.BoolValue = value;
}

void AnimationController::SetFloat(const std::string& name, float value)
{
	if (!m_Parameters.contains(name))
	{
		Logger::Log("parameter by this name doesn't exist", "Animator", LOG_TYPE::WARNING);
		return;
	}
	auto& param = m_Parameters.at(name);
	param.FloatValue = value;
}

void AnimationController::SetTrigger(const std::string& name)
{
	if (!m_Parameters.contains(name))
	{
		Logger::Log("parameter by this name doesn't exist", "Animator", LOG_TYPE::WARNING);
		return;
	}
	auto& param = m_Parameters.at(name);
	param.Triggered = true;
}

void AnimationController::Play(const std::string& stateName)
{
	if (m_States.contains(stateName))
	{
		m_CurrentState = stateName;
		m_StateTimer = 0.0f;
		m_CurrentFrame = 0;
	}
}

void AnimationController::Update(float dt)
{
	if (!m_States.contains(m_CurrentState))
		return;

	auto& state = m_States[m_CurrentState];
	auto& clip = state.Clip;
	
	if (clip->m_Frames.empty())
		return;

	m_StateTimer += dt;
	float time = 0.0f;

	// find which frame we should be on...
	for (size_t i = 0; i < clip->m_Frames.size(); i++)
	{
		time += clip->m_Frames[i].Duration;
		if (m_StateTimer < time)
		{
			m_CurrentFrame = i;
			break;
		}
	}

	if (m_StateTimer >= clip->m_TotalDuration)
	{
		if (clip->m_Loop)
		{
			m_StateTimer = 0;
		}
	}

	EvaluateTransitions();
}

AnimationFrame AnimationController::GetCurrentFrame() const
{
	const auto& clip = m_States.at(m_CurrentState).Clip;
	return clip->m_Frames[m_CurrentFrame];
}

bool AnimationController::EvaluateTransitions()
{
	if (!m_States.contains(m_CurrentState))
		return false;

	auto& state = m_States[m_CurrentState];

	for (auto& transition : state.Transitions)
	{
		bool conditionsMet = true;

		for (auto& condition : transition.Conditions)
		{
			if (!m_Parameters.contains(condition.ParameterName))
			{
				conditionsMet = false;
				Logger::Log("condition doesnt have param: " + condition.ParameterName, "animator");
				break;
			}

			Logger::Log("condition has param: " + condition.ParameterName, "animator");

			auto& param = m_Parameters.at(condition.ParameterName);

			switch (condition.ConditionType)
			{
			case AnimatorConditionType::BoolTrue:
				if (!param.BoolValue) conditionsMet = false;
				break;
			case AnimatorConditionType::BoolFalse:
				if (param.BoolValue) conditionsMet = false;
				break;
			case AnimatorConditionType::FloatGreater:
				if (!(param.FloatValue > condition.Threshold)) conditionsMet = false;
				break;
			case AnimatorConditionType::FloatLess:
				if (!(param.FloatValue < condition.Threshold)) conditionsMet = false;
				break;
			case AnimatorConditionType::Trigger:
				if (!param.Triggered) conditionsMet = false;
				break;
			}
		}

		if (conditionsMet)
		{
			Play(transition.ToState);

			// reset triggers
			for (auto& condition : transition.Conditions)
			{
				if (!m_Parameters.contains(condition.ParameterName))
				{
					break;
				}
				auto& param = m_Parameters.at(condition.ParameterName);
				if (param.Type == AnimatorParameterType::Trigger)
					param.Triggered = false;
			}

			return true;
		}
	}

	return false;
}
