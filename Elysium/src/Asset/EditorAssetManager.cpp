#include "EditorAssetManager.h"

#include "core/Logger.h"

#include "AssetImporter.h"
#include "Project/Project.h"

#include <yaml-cpp/yaml.h>

#include <fstream>


static std::map<std::filesystem::path, AssetType> s_AssetExtensionMap = {
	{".elysium", AssetType::Scene},
	{".png", AssetType::Texture2D},
	{".esmspritesheet", AssetType::SpriteSheet}
};

static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension)
{
	if (s_AssetExtensionMap.find(extension) == s_AssetExtensionMap.end())
	{
		Logger::Log("file extension not valid", "Asset Manager", LOG_TYPE::WARNING);
		return AssetType::None;
	}
	return s_AssetExtensionMap.at(extension);
}

YAML::Emitter& operator << (YAML::Emitter& out, const std::string_view& v)
{
	out << std::string(v.data(), v.size());
	return out;
}

bool EditorAssetManager::IsAssetHandleValid(AssetHandle handle) const
{
	return handle != 0 && m_AssetRegistry.find(handle) != m_AssetRegistry.end();
}

bool EditorAssetManager::IsAssetLoaded(AssetHandle handle) const
{
	return m_LoadedAssets.find(handle) != m_LoadedAssets.end();
}

AssetType EditorAssetManager::GetAssetType(AssetHandle handle) const
{
	if (!IsAssetHandleValid(handle))
		return AssetType::None;
	return m_AssetRegistry.at(handle).Type;
}

void EditorAssetManager::ImportAsset(const std::filesystem::path& filepath)
{
	AssetHandle handle; // generate new handle
	AssetMetadata metadata;
	metadata.FilePath = filepath;
	metadata.Type = GetAssetTypeFromFileExtension(filepath.extension());
	// assert: type != none
	std::shared_ptr<Asset> asset = AssetImporter::ImportAsset(handle, metadata);
	if (asset)
	{
		asset->Handle = handle;
		m_LoadedAssets[handle] = asset;
		m_AssetRegistry[handle] = metadata;
		SerializeAssetRegistry(); // TODO: should we do this everytime a new asset is imported ??
	}
}

const AssetMetadata& EditorAssetManager::GetMetadata(AssetHandle handle) const
{
	static AssetMetadata s_NullMetadata;
	auto it = m_AssetRegistry.find(handle);
	if (it == m_AssetRegistry.end())
		return s_NullMetadata;
	return it->second;
}

const std::filesystem::path& EditorAssetManager::GetFilePath(AssetHandle handle) const
{
	return GetMetadata(handle).FilePath;
}

const bool EditorAssetManager::AssetExistsAtFilePath(const std::filesystem::path& filepath) const
{
	for (const auto& [handle, metadata] : m_AssetRegistry)
	{
		if (metadata.FilePath == filepath)
			return true;
	}
	return false;
}

const AssetHandle EditorAssetManager::GetAssetHandle(const std::filesystem::path& filepath) const
{
	for (const auto& [handle, metadata] : m_AssetRegistry)
	{
		if (metadata.FilePath == filepath)
			return handle;
	}
	//ESM_Error/Assert: this should never happen...
	return 0;
}

std::shared_ptr<Asset> EditorAssetManager::GetAsset(AssetHandle handle)
{
	if (!IsAssetHandleValid(handle))
	{
		return nullptr;
	}

	std::shared_ptr<Asset> asset;
	if (IsAssetLoaded(handle))
	{
		asset = m_LoadedAssets.at(handle);
	}
	else
	{
		const AssetMetadata& metadata = GetMetadata(handle);
		asset = AssetImporter::ImportAsset(handle, metadata);
		if (!asset)
		{
			// log error: asset import failed
		}
		m_LoadedAssets[handle] = asset;
	}
	return asset;
}

void EditorAssetManager::SerializeAssetRegistry()
{
	auto path = Project::GetActiveAssetRegistryPath();

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "AssetRegistry" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Handle" << YAML::Value << handle;
			out << YAML::Key << "FilePath" << YAML::Value << metadata.FilePath.generic_string();
			out << YAML::Key << "Type" << YAML::Value << AssetTypeToString(metadata.Type);
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap; // root
	}
	std::ofstream fout(path);
	fout << out.c_str();
}

bool EditorAssetManager::DeserializeAssetRegistry()
{
	auto path = Project::GetActiveAssetRegistryPath();
	YAML::Node data;
	try
	{
		data = YAML::LoadFile(path.string());
	}
	catch (YAML::Exception e)
	{
		// log error , failed to load project file
		return false;
	}
	auto rootNode = data["AssetRegistry"];
	if (!rootNode)
		return false;

	for (const auto& node : rootNode)
	{
		AssetHandle handle = node["Handle"].as<uint64_t>();
		auto& metadata = m_AssetRegistry[handle];
		metadata.FilePath = node["FilePath"].as<std::string>();
		metadata.Type = AssetTypeFromString(node["Type"].as<std::string>());
	}

	return true;
}
