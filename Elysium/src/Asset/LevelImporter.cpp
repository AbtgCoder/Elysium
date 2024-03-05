#include "LevelImporter.h"

#include "Project/Project.h"
#include "Level/LevelSerializer.h"

std::shared_ptr<Level> LevelImporter::ImportLevel(AssetHandle handle, const AssetMetadata& metadata)
{
	return LoadLevel(Project::GetActiveAssetDirectory() / metadata.FilePath);
}

std::shared_ptr<Level> LevelImporter::LoadLevel(const std::filesystem::path& path)
{
	std::shared_ptr<Level> level = std::make_shared<Level>();
	LevelSerializer serializer(level);
	serializer.Deserialize(path);
	return level;
}

void LevelImporter::SaveLevel(std::shared_ptr<Level> level, const std::filesystem::path& path)
{
	LevelSerializer serializer(level);
	serializer.Serialize(Project::GetActiveAssetDirectory() / path);
}
