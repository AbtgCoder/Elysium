#include "SpriteSheetImporter.h"

#include "Project/Project.h"
#include "Animation/SpriteSheetSerializer.h"

std::shared_ptr<SpriteSheet> SpriteSheetImporter::ImportSpriteSheet(AssetHandle handle, const AssetMetadata& metadata)
{
	auto ss = LoadSpriteSheet(Project::GetActiveAssetDirectory() / metadata.FilePath);
	ss->Handle = handle;
	return ss;
}

std::shared_ptr<SpriteSheet> SpriteSheetImporter::LoadSpriteSheet(const std::filesystem::path& path)
{
	std::shared_ptr<SpriteSheet> spriteSheet = std::make_shared<SpriteSheet>();
	SpriteSheetSerializer serializer(spriteSheet);
	serializer.Deserialize(path);
	return spriteSheet;
}

void SpriteSheetImporter::SaveSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet, const std::filesystem::path& path)
{
	SpriteSheetSerializer serializer(spriteSheet);
	serializer.Serialize(Project::GetActiveAssetDirectory() / path);
}
