#include "AssetImporter.h"

#include "SceneImporter.h"
#include "TextureImporter.h"
#include "SpriteSheetImporter.h"
#include "AnimationImporter.h"

#include <functional>
#include <map>

using AssetImportFunction = std::function<std::shared_ptr<Asset>(AssetHandle, const AssetMetadata&)>;
static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
	{AssetType::Scene, SceneImporter::ImportScene}, 
	{AssetType::Texture2D, TextureImporter::ImportTexture2D}, // other asset imports
	{AssetType::SpriteSheet, SpriteSheetImporter::ImportSpriteSheet},
	{AssetType::AnimationClip, AnimationImporter::ImportAnimationClip}
};

std::shared_ptr<Asset> AssetImporter::ImportAsset(AssetHandle handle, const AssetMetadata& metadata)
{
	if (s_AssetImportFunctions.find(metadata.Type) == s_AssetImportFunctions.end())
	{
		// log error: no importer available for this asset type
		return nullptr;
	}

	return s_AssetImportFunctions.at(metadata.Type)(handle, metadata);
}
