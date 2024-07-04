#include "Layer.h"

Layer::Layer()
{
}

Layer::Layer(Application* Application)
	: m_game(Application)
{
}

void Layer::simulate(int s)
{
}

void Layer::doAction(const Action& action)
{
}

void Layer::registerAction(int inputKey, const std::string& actionName)
{
	m_actionMap[inputKey] = actionName;
}



size_t Layer::width() const
{
	return size_t(1500);
}

size_t Layer::height() const
{
	return size_t(720);
}

size_t Layer::currentFrame() const
{
	return m_currentFrame;
}

bool Layer::hasEnded() const
{
	return m_hasEnded;
}

const ActionMap& Layer::getActionMap() const
{
	return m_actionMap;
}


void Layer::setPaused(bool paused)
{
	m_paused = paused;
}
