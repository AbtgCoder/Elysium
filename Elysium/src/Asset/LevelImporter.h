#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Level/Level.h"

class LevelImporter
{
public:
	// Asset Metadata filepath is relative to project asset directory
	static std::shared_ptr<Level> ImportLevel(AssetHandle handle, const AssetMetadata& metadata);
	// Load from filepath
	static std::shared_ptr<Level> LoadLevel(const std::filesystem::path& path);

	static void SaveLevel(std::shared_ptr<Level> level, const std::filesystem::path& path);
};