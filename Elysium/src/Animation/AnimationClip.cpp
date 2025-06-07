#include "AnimationClip.h"

void AnimationClip::AddFrame(const glm::vec2& uvMin, const glm::vec2& uvMax, float duration)
{
	m_Frames.push_back({ uvMin, uvMax, duration });
	m_TotalDuration += duration;
}
