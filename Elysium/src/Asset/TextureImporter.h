#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Core/Texture.h"

class TextureImporter
{
public:
	// asset metadata filepath is relative to project directory
	static std::shared_ptr<Texture> ImportTexture(AssetHandle handle, const AssetMetadata& metadata);
	// load texture directory from filesystem
	static std::shared_ptr<Texture> LoadTexture(const std::filesystem::path& path);
};
