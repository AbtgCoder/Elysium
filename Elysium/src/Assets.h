#pragma once

#include <map>
#include <string>
#include <SFML/Graphics.hpp>
//#include <SFML/Audio.hpp>
#include <iostream>
#include "Animation.h"
#include <unordered_map>
#include <SFML/OpenGL.hpp>


class Assets
{
public:
	void addTexture(std::string name, std::string path);
	void addAnimation(std::string name, std::string path);
	void addAnimation(std::string name, std::string path, size_t frameCount, size_t speed);
	//void addSound(std::string name, std::string path);
	void addFont(std::string name, std::string path);

	const sf::Texture& getTexture(const std::string& name);
	const Animation& getAnimation(const std::string& name) const;
	//sf::Sound& getSound(std::string name);
	const sf::Font& getFont(const std::string& name) const;
	//std::unordered_map<std::string, GLuint> m_textureIDs;
	std::map<std::string, sf::Texture> m_textures;



private:
	std::map<std::string, Animation> m_animations;
	//::map<std::string, sf::Sound> m_sounds;
	std::map<std::string, sf::Font> m_fonts;
};