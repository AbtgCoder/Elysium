#include "Scene.h"

Scene::Scene()
{
}

Scene::Scene(GameEngine* gameEngine)
	: m_game(gameEngine)
{
}

void Scene::simulate(int s)
{
}

void Scene::doAction(const Action& action)
{
}

void Scene::registerAction(int inputKey, const std::string& actionName)
{
	m_actionMap[inputKey] = actionName;
}



size_t Scene::width() const
{
	return size_t(1500);
}

size_t Scene::height() const
{
	return size_t(720);
}

size_t Scene::currentFrame() const
{
	return m_currentFrame;
}

bool Scene::hasEnded() const
{
	return m_hasEnded;
}

const ActionMap& Scene::getActionMap() const
{
	return m_actionMap;
}


void Scene::setPaused(bool paused)
{
	m_paused = paused;
}
