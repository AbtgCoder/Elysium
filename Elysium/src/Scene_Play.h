#pragma once

#include "Scene.h"


class Scene_Play : public Scene
{
	struct PlayerConfig
	{ 
		float X, Y, CX, CY, SPEED, MAXSPEED, JUMP, GRAVITY;
		std::string WEAPON;
	};


public:
	Scene_Play(GameEngine* gameEngine, std::string& levelPath);
	void update();

	// Systems
	void sAnimation();
	void sMovement();
	void sEnemySpawner();
	void sCollision();
	void sCamera();
	void sRender();
	void sDoAction(const Action& action);
	void onEnd();


protected:
	std::string m_levelPath;
	std::shared_ptr<Entity> m_player;
	PlayerConfig m_playerConfig;
	bool m_drawTextures = true;
	bool m_drawCollision = false;
	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };
	sf::Text m_gridText;
	sf::View m_gameView;
	int m_score = 0;
	size_t m_lastEnemySpawnTime = 0;
	size_t m_enemySpawnInterval = 60;
	size_t m_numEnemiesAllowed = 3;
	size_t m_numEnemiesSpawned = 0;


	void init(const std::string& levelPath);
	Vec2 gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity);
	Vec2 gridToMidPixel(float gridX, float gridY);
	void loadLevel(const std::string& filename);
	void spawnPlayer();



};