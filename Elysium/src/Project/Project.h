#pragma once

#include <string>
#include <filesystem>

#include "Asset/RuntimeAssetManager.h"
#include "Asset/EditorAssetManager.h"

struct ProjectConfig
{
	std::string Name = "Untitled";

	AssetHandle StartScene;

	std::filesystem::path AssetDirectory; // relative to ProjectDirectory
	std::filesystem::path AssetRegistryPath; // relative to AssetDirectory

	std::filesystem::path ScriptModulePath; // relative to ProjectDirectory (TODO: maybe should be relative to AssetDirectory)

	AssetHandle lastOpenedScene; // for editor only (TODO: maybe find a better way to do this)
};

class Project
{
public:
	//~Project() { std::cout << "project destroyed!!\n"; }

	const std::filesystem::path& GetProjectDirectory() { return m_ProjectDirectory; }
	std::filesystem::path GetAssetDirectory() { return GetProjectDirectory() / s_ActiveProject->m_Config.AssetDirectory; }
	std::filesystem::path GetAssetRegistryPath() { return GetAssetDirectory() / s_ActiveProject->m_Config.AssetRegistryPath; }
	std::filesystem::path GetScriptModulePath() { return GetProjectDirectory() / s_ActiveProject->m_Config.ScriptModulePath; }

	static const std::filesystem::path& GetActiveProjectDirectory()
	{
		// assert s_ActiveProject
		return s_ActiveProject->GetProjectDirectory();
	}
	static const std::filesystem::path GetActiveAssetDirectory()
	{
		// assert s_ActiveProject
		return s_ActiveProject->GetAssetDirectory();
	}
	static const std::filesystem::path GetActiveAssetRegistryPath()
	{
		// assert s_ActiveProject
		return s_ActiveProject->GetAssetRegistryPath();
	}
	static const std::filesystem::path GetActiveScriptModulePath()
	{
		// assert s_ActiveProject
		return s_ActiveProject->GetScriptModulePath();
	}

	static void SetLastOpenedScene(AssetHandle handle) { s_ActiveProject->m_Config.lastOpenedScene = handle; }

	ProjectConfig& GetConfig() { return m_Config; }

	static std::shared_ptr<Project> GetActive() { return s_ActiveProject; }
	std::shared_ptr<AssetManagerBase> GetAssetManager() { return m_AssetManager; }
	std::shared_ptr<RuntimeAssetManager> GetRuntimeAssetManager() { return std::static_pointer_cast<RuntimeAssetManager>(m_AssetManager); }
	std::shared_ptr<EditorAssetManager> GetEditorAssetManager() { return std::static_pointer_cast<EditorAssetManager>(m_AssetManager); }

	static std::shared_ptr<Project> New();
	static std::shared_ptr<Project> New(const std::string& name, const std::string& location);
	static std::shared_ptr<Project> Load(const std::filesystem::path& path);
	static bool SaveActive(const std::filesystem::path& path);
private:
	ProjectConfig m_Config;
	std::filesystem::path m_ProjectDirectory;
	std::shared_ptr<AssetManagerBase> m_AssetManager;

	inline static std::shared_ptr<Project> s_ActiveProject;
};