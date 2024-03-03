#include "AssetImporter.h"

#include "LevelImporter.h"
#include "TextureImporter.h"

#include <functional>
#include <map>

using AssetImportFunction = std::function<std::shared_ptr<Asset>(AssetHandle, const AssetMetadata&)>;
static std::map<AssetType, AssetImportFunction> s_AssetImportFunctions = {
	{AssetType::Level, LevelImporter::ImportLevel}, 
	{AssetType::Texture, TextureImporter::ImportTexture} // other asset imports
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
