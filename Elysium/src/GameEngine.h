#pragma once

#include "Scene.h"
#include "Assets.h"
#include <memory>

typedef std::map < std::string, std::shared_ptr<Scene>> SceneMap;

class GameEngine
{
public:
	GameEngine(const std::string& path);

	void changeScene(const std::string& sceneName, std::shared_ptr<Scene> scene, bool endCurrentScene = false);


	void quit();
	void run();

	sf::RenderWindow& window();
	sf::Clock m_deltaClock;
	const Assets& assets() const;
	bool isRunning();
	const SceneMap& scenes() const;
	 
protected:
	sf::RenderWindow m_window;
	Assets m_assets;

	std::string m_currentScene;
	SceneMap m_sceneMap;
	size_t m_simulationSpeed = 1;
	bool m_running = true;

	// init-> load all assets
	void init(const std::string& path);
	void update();

	void sUserInput();

	std::shared_ptr<Scene> currentScene();
};