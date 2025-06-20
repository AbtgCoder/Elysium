#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Animation/AnimationController.h"

class AnimationImporter
{
public:
	static std::shared_ptr<AnimationClip> ImportAnimationClip(AssetHandle handle, const AssetMetadata& metadata);

	static std::shared_ptr<AnimationClip> LoadAnimationClip(const std::filesystem::path& path);

	static void SaveAnimationClip(std::shared_ptr<AnimationClip> animationClip, const std::filesystem::path& path);
};
