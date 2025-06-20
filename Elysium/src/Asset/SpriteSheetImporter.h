#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Animation/SpriteSheet.h"

class SpriteSheetImporter
{
public:
	static std::shared_ptr<SpriteSheet> ImportSpriteSheet(AssetHandle handle, const AssetMetadata& metadata);

	static std::shared_ptr<SpriteSheet> LoadSpriteSheet(const std::filesystem::path& path);

	static void SaveSpriteSheet(std::shared_ptr<SpriteSheet> spriteSheet, const std::filesystem::path& path);
};