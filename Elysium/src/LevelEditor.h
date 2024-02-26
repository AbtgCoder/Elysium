#pragma once

#include "Scene.h"

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

	void sDockingRender();
protected:
	sf::View m_levelView;
	Vec2 m_levelViewCenter;
	float m_levelViewZoom = 1.0f;
	bool m_altPressed = false;
	Vec2 m_lastLevelViewPos;
	bool m_levelViewMoving = false;

	sf::RectangleShape m_gameBG;

	sf::RenderTexture m_rt{};
	Vec2 m_viewportSize;
	std::pair<Vec2, Vec2> m_viewportBounds;

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
	bool m_enableDragging = true;

	bool m_playAnimation = false;

	void setImGuiStyle();
	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;
	sf::Texture m_DirectoryIcon;
	sf::Texture m_FileIcon;
	void contentBrowserGUI();
	
	void drawEntityNode(std::shared_ptr<Entity> entity);
	void entityManagerGUI();
	void entityInspectorGUI();
	template<typename T, typename... TArgs>
	void DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs);


	Vec2 worldToGrid(std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY);
	void snapToGrid(std::shared_ptr<Entity> entity);
	Vec2 windowToWorld(const Vec2& windowPos) const;

	std::vector<Vec2> generatePolygonColliderVertices(std::shared_ptr<Entity> entity);

	bool loadLevel(const std::filesystem::path& filepath);
	void loadAssets(const std::string& assetDir);
	void saveLevel();
	void spawnEntity(const std::string& name, const sf::Texture& tex);

	void init();
	void update();
	void onEnd();
	void sDoAction(const Action& action);
};