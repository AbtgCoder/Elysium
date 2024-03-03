#include "Asset.h"

std::string_view AssetTypeToString(AssetType type)
{
	switch (type)
	{
	case AssetType::None: return "AssetType::None";
	case AssetType::Level: return "AssetType::Level";
	case AssetType::Texture: return "AssetType::Texture";
	}
	return "AssetType::<Invalid>";
}

AssetType AssetTypeFromString(std::string_view assetType)
{
	if (assetType == "AssetType::None") return AssetType::None;
	if (assetType == "AssetType::Level") return AssetType::Level;
	if (assetType == "AssetType::Texture") return AssetType::Texture;
	return AssetType::None;
}
