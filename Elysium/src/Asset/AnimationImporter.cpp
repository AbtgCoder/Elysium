#include "AnimationImporter.h"

#include "Project/Project.h"
#include "Animation/AnimationSerializer.h"

std::shared_ptr<AnimationClip> AnimationImporter::ImportAnimationClip(AssetHandle handle, const AssetMetadata& metadata)
{
	auto clip = LoadAnimationClip(Project::GetActiveAssetDirectory() / metadata.FilePath);
	clip->Handle = handle; //NOTE: seems very important, as when we get the asset through the asset manager, it should have the correct handle for later use 
	return clip;
}

std::shared_ptr<AnimationClip> AnimationImporter::LoadAnimationClip(const std::filesystem::path& path)
{
	std::shared_ptr<AnimationClip> clip = std::make_shared<AnimationClip>();
	AnimationSerializer serializer(clip);
	serializer.Deserialize(path);
	return clip;
}

void AnimationImporter::SaveAnimationClip(std::shared_ptr<AnimationClip> animationClip, const std::filesystem::path& path)
{
	AnimationSerializer serializer(animationClip);
	serializer.Serialize(Project::GetActiveAssetDirectory() / path);
}
