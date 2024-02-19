#include "Assets.h"

void Assets::addTexture(std::string name, std::string path)
{
	sf::Texture texture;
	if (!texture.loadFromFile(path))
	{
		std::cerr << "couldn't load texture!";
	}
    texture.setSmooth(true);
	m_textures[name] = texture;

  /*  GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture.getSize().x, texture.getSize().y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.copyToImage().getPixelsPtr());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_textureIDs[name] = textureID;*/


}


void Assets::addAnimation(std::string name, std::string path)
{
    addTexture(name, path);
    m_animations[name] = Animation(name, m_textures[name]);
}

void Assets::addAnimation(std::string name, std::string path, size_t frameCount, size_t speed)
{
    addTexture(name, path);
    m_animations[name] = Animation(name, m_textures[name], frameCount, speed);
}



void Assets::addFont(std::string name, std::string path)
{
    sf::Font font;
    if (!font.loadFromFile("../../../Assets/fonts/tech.ttf"))
    {
        std::cerr << "couldn't load font!";
    }
    m_fonts[name] = font;
}

const sf::Texture& Assets::getTexture(const std::string& name) const
{
	return m_textures.at(name);
}

const Animation& Assets::getAnimation(const std::string& name) const
{
    auto it = m_animations.find(name);
    if (it != m_animations.end()) {
        return it->second; 
    }
    else {
        std::cerr << "Animation " << name << " not found";
    }
}


const sf::Font& Assets::getFont(const std::string& name) const
{
    return m_fonts.at(name);
}
