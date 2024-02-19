#pragma once

#include <SFML/Graphics.hpp>

#include <filesystem>

class ContentBrowserPanel
{
public:
	ContentBrowserPanel(std::filesystem::path assetDirectory);

	void OnImGuiRender();
private:
	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;
	sf::Texture m_DirectoryIcon;
	sf::Texture m_FileIcon;
};