#pragma once

#include "Scene.h"

#include <filesystem>

enum GIZMO_OPERATION
{
	TRANSLATE,
	SCALE,
	ROTATE, // TODO: add rotation gizmo!!
};

class LevelEditor : public Scene
{
public:
	LevelEditor(GameEngine* gameEngine = nullptr);
	void sGUI();
	void sAnimation();
	void sCollision();
	void sRender();

	void sDockingRender();
protected:
	// Level Editor Camera and Camera Controller stuff
	sf::View m_levelView;
	Vec2 m_levelViewCenter;
	float m_levelViewZoom = 1.0f;
	bool m_altPressed = false;
	Vec2 m_lastLevelViewPos;
	bool m_levelViewMoving = false;

	// Main Rendering Viewport
	sf::RenderTexture m_rt{};
	Vec2 m_viewportSize;
	std::pair<Vec2, Vec2> m_viewportBounds;

	// Gizmo stuff
	bool m_gizmoHoverX = false;
	bool m_gizmoSelectX = false;
	Vec2 m_lastGizmoPosX;
	bool m_gizmoHoverY = false;
	bool m_gizmoSelectY = false;
	Vec2 m_lastGizmoPosY;
	float m_scalingFactor = 0.05f;
	int m_gizmoType = GIZMO_OPERATION::TRANSLATE;

	// Assets : Only Textures for now!
	std::map<std::string, sf::Texture> m_assets;
	
	// Debug stuff
	bool m_drawCollision = false;
	sf::RectangleShape m_collisionRect;
	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };
	sf::Text m_gridText;
	sf::RectangleShape m_gridRect;

	Vec2 m_mousePos;
	std::shared_ptr<Entity> m_inspectedEntity = nullptr;
	sf::CircleShape m_cursorDot;

	// ImGui
	void setImGuiStyle();
	// ImGui - Content Browser Panel
	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;
	sf::Texture m_DirectoryIcon;
	sf::Texture m_FileIcon;
	void contentBrowserGUI();
	// ImGui - Entity Manager and Entity Inspector Panels
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