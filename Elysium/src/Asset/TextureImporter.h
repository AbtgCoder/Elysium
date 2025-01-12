#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Renderer/Texture.h"

class TextureImporter
{
public:
	// asset metadata filepath is relative to project directory
	static std::shared_ptr<Texture2D> ImportTexture2D(AssetHandle handle, const AssetMetadata& metadata);
	// load texture directory from filesystem
	static std::shared_ptr<Texture2D> LoadTexture2D(const std::filesystem::path& path);
};
