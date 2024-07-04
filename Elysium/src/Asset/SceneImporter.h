#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

#include "Scene/Scene.h"

class SceneImporter
{
public:
	// Asset Metadata filepath is relative to project asset directory
	static std::shared_ptr<Scene> ImportScene(AssetHandle handle, const AssetMetadata& metadata);
	// Load from filepath
	static std::shared_ptr<Scene> LoadScene(const std::filesystem::path& path);

	static void SaveScene(std::shared_ptr<Scene> Scene, const std::filesystem::path& path);
};