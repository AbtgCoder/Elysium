#include "TextureImporter.h"

#include "Project/Project.h"

std::shared_ptr<Texture> TextureImporter::ImportTexture(AssetHandle handle, const AssetMetadata& metadata)
{
	return LoadTexture(Project::GetActiveAssetDirectory() / metadata.FilePath);
}

std::shared_ptr<Texture> TextureImporter::LoadTexture(const std::filesystem::path& path)
{
	// sf texture load from file ?? 
	sf::Texture tex;
	if (!tex.loadFromFile(path.string()))
	{
		return std::make_shared<Texture>();
	}
	return std::make_shared<Texture>(tex);
}
