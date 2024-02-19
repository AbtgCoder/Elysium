#include "Animation.h"
#include <cmath>

Animation::Animation()
{
}

Animation::Animation(const std::string& name, const sf::Texture& t)
	: Animation(name, t, 1, 0)
{

}

Animation::Animation(const std::string& name, const sf::Texture& t, size_t frameCount, size_t speed)
	: m_name(name)
	, m_sprite(t)
	, m_frameCount(frameCount)
	, m_currentFrame(0)
	, m_speed(speed)
{
	m_size = Vec2((float)t.getSize().x/frameCount, (float)t.getSize().y);
	m_sprite.setOrigin(m_size.x/2.0f, m_size.y/2.0f);
	m_sprite.setTextureRect(sf::IntRect(std::floor(m_currentFrame)*m_size.x, 0, m_size.x, m_size.y));
}


void Animation::update()
{
	// actual animation logic
	m_currentFrame++;
	if (m_speed != 0 && !hasEnded())
	{
		m_sprite.setTextureRect(sf::IntRect(((int)(m_currentFrame / m_speed) % m_frameCount) * m_size.x, 0, m_size.x, m_size.y));
	}
	
}

bool Animation::hasEnded()
{
	if ( m_currentFrame >= m_speed && (int)(m_currentFrame / m_speed) % m_frameCount == 0)
	{
		return true;
	}
	return false;
}

std::string& Animation::getName()
{
	return m_name;
}

Vec2& Animation::getSize()
{
	return m_size;
}


sf::Sprite& Animation::getSprite()
{
	return m_sprite;
}

size_t Animation::getSpeed()
{
	return m_speed;
}

size_t Animation::getFrameCount()
{
	return m_frameCount;
}



