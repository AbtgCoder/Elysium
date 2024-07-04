#pragma once

#include "Layer.h"
#include <memory>

typedef std::map<std::string, std::shared_ptr<Layer>> LayerMap;

class Application
{
public:
	Application(const std::string& name);

	void changeLayer(const std::string& LayerName, std::shared_ptr<Layer> Layer, bool endCurrentLayer = false);


	void quit();
	void run();

	sf::RenderWindow& window();
	sf::Clock m_deltaClock;
	bool isRunning();
	const LayerMap& Layers() const;
	 
protected:
	sf::RenderWindow m_window;

	std::string m_currentLayer;
	LayerMap m_LayerMap;
	size_t m_simulationSpeed = 1;
	bool m_running = true;

	void init(const std::string& path);
	void update(float dt);

	void sUserInput();

	std::shared_ptr<Layer> currentLayer();
};