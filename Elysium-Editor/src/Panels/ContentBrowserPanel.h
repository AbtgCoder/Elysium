#pragma once

#include "Project/Project.h"
#include "Renderer/Texture.h"
#include "ThumbnailCache.h"

#include <filesystem>
#include <map>
#include <set>

class ContentBrowserPanel
{
public:
	ContentBrowserPanel(std::shared_ptr<Project> project);

	void OnImGuiRender();
//private:
	void RefreshAssetTree();
private:
	std::shared_ptr<Project> m_Project;
	std::shared_ptr<ThumbnailCache> m_ThumbnailCache;
	
	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;

	std::shared_ptr<Texture2D> m_DirectoryIcon;
	std::shared_ptr<Texture2D> m_FileIcon;

	std::string m_SearchQuery = "";

};