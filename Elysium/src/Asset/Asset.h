#pragma once

#include "Core/UUID.h"

using AssetHandle = Elysium::UUID;

enum class AssetType : uint16_t
{
	None = 0,
	Texture,
	Scene,
};

std::string_view AssetTypeToString(AssetType type);
AssetType AssetTypeFromString(std::string_view assetType);

class Asset
{
public:
	AssetHandle Handle; // Generate handle

	virtual AssetType GetType() const = 0;
};