#include "AnimationController.h"

void AnimationController::AddState(const std::string& name, std::shared_ptr<AnimationClip> clip)
{
	m_States[name] = { name, clip };
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
		//TODO: transitions go here ??

		if (clip->m_Loop)
		{
			m_StateTimer = 0;
		}
	}
}

AnimationFrame AnimationController::GetCurrentFrame() const
{
	const auto& clip = m_States.at(m_CurrentState).Clip;
	return clip->m_Frames[m_CurrentFrame];
}
