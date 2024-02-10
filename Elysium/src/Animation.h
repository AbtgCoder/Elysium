#pragma once

#include "Vec2.h"
#include <SFML/Graphics.hpp>

class Animation
{
public:
	void update();
	bool hasEnded();
	std::string& getName();
	Vec2& getSize();
	sf::Sprite& getSprite();
	size_t getSpeed();
	size_t getFrameCount();


	Animation();
	Animation(const std::string& name, const sf::Texture& t);
	Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed);
	//Animation(Animation& other);
private:
	sf::Sprite m_sprite;
	size_t m_frameCount = 1; // number of frames of animation
	size_t m_currentFrame = 0;
	size_t m_speed = 0;
	Vec2 m_size = { 1,1 }; // size of animation frame
	std::string m_name = "none";
};