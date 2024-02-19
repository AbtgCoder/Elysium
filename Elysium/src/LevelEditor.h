#pragma once

#include "Scene.h"
#include "Panels/LevelHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/EntityInspectorPanel.h"

#include <filesystem>

class LevelEditor : public Scene
{
public:
	LevelEditor(GameEngine* gameEngine = nullptr);
	void sGUI();
	void sAnimation();
	void sCollision();
	void sRender();
	void sDrag();
protected:
	sf::View m_gameView;
	sf::RectangleShape m_gameBG;

	std::map<std::string, sf::Texture> m_assets;
	
	bool m_drawCollision = false;
	sf::RectangleShape m_collisionRect;

	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };
	sf::Text m_gridText;
	sf::RectangleShape m_gridRect;

	Vec2 m_mousePos;
	std::shared_ptr<Entity> m_inspectedEntity = nullptr;
	sf::CircleShape m_cursorDot;
	bool m_enableDragging = false;

	bool m_playAnimation = true;

	void setImGuiStyle();

	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;
	sf::Texture m_DirectoryIcon;
	sf::Texture m_FileIcon;
	void contentBrowserGUI();

	// Panels
	LevelHierarchyPanel m_LevelHierarchyPanel;
	std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
	EntityInspectorPanel m_EntityInspectorPanel;

	void drawEntityNode(std::shared_ptr<Entity> entity);
	void entityManagerGUI();
	void entityInspectorGUI();
	template<typename T>
	void DisplayAddComponentEntry(const std::string& entryName);

	Vec2 worldToGrid(std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY);
	void snapToGrid(std::shared_ptr<Entity> entity);
	Vec2 windowToWorld(const Vec2& windowPos) const;

	void loadLevel();
	void loadAssets(const std::string& assetDir);
	void saveLevel();
	void spawnEntity(const std::string& name, const sf::Texture& tex);

	void init();
	void update();
	void onEnd();
	void sDoAction(const Action& action);
};
