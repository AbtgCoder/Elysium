#pragma once


#include "AnimationController.h"

#include <filesystem>

class AnimationSerializer
{
public:
	AnimationSerializer(const std::shared_ptr<AnimationClip>& animtionClip);

	void Serialize(const std::filesystem::path& filepath);

	bool Deserialize(const std::filesystem::path& filepath);

private:
	std::shared_ptr<AnimationClip> m_AnimationClip;
};