#pragma once

#include "Layer.h"
#include <memory>

typedef std::map<std::string, std::shared_ptr<Layer>> LayerMap;

int main(int argc, char** argv);

class Application
{
public:
	Application(const std::string& name);
	~Application();

	void changeLayer(const std::string& LayerName, std::shared_ptr<Layer> Layer, bool endCurrentLayer = false);

	static Application& Get() { return *s_Instance; }

	void quit();
	void run();

	sf::RenderWindow& window();
	sf::Clock m_deltaClock;
	bool isRunning();
	const LayerMap& Layers() const;
	 
protected:
	static Application* s_Instance;
	friend int ::main(int argc, char** argv);

	sf::RenderWindow m_window;

	std::string m_currentLayer;
	LayerMap m_LayerMap;
	size_t m_simulationSpeed = 1;
	bool m_running = true;

	void init(const std::string& path);
	void update(float ts);

	void sUserInput();

	std::shared_ptr<Layer> currentLayer();
};

// to be defined in Client 
Application* CreateApplication();