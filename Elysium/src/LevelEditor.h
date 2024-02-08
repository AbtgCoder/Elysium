#pragma once


#include "Scene.h"

class LevelEditor : public Scene
{
public:
	LevelEditor(GameEngine* gameEngine = nullptr);
	void sGUI();
	void sRender();
	void sDrag();
protected:
	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };
	std::map<std::string, sf::Texture> m_assets;
	Vec2 m_mousePos;

	Vec2 worldToGrid(std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY);
	void snapToGrid(std::shared_ptr<Entity> entity);
	Vec2 windowToWorld(const Vec2& window) const;

	void loadLevel();
	void loadAssets(const std::string& assetDir);
	void saveLevel();
	void spawnEntity(const std::string& name, const sf::Texture& tex);

	void init();
	void update();
	void onEnd();
	void sDoAction(const Action& action);
};