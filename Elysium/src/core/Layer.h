#pragma once

#include "Action.h"

#include <memory>

class Application; 

typedef std::map<int, std::string> ActionMap;

class Layer
{
public:
	Layer();
	Layer(Application* Application);

	virtual void update(float ts) = 0;
	virtual void sDoAction(const Action& action) = 0;
	virtual void sRender() = 0;

	void simulate(int n); // call derived's update() n times 
	virtual void doAction(const Action& action);
	void registerAction(int inputKey, const std::string& actionName);

	size_t width() const;
	size_t height() const;
	size_t currentFrame() const;

	bool hasEnded() const;
	const ActionMap& getActionMap() const;

protected:
	Application* m_game=nullptr;
	ActionMap m_actionMap;
	bool m_paused = false;
	bool m_hasEnded = false;
	size_t m_currentFrame = 0;

	virtual void onEnd() = 0;
	void setPaused(bool paused);
};