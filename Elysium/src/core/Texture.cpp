#include "Texture.h"

Texture::Texture(const sf::Texture& texture)
	: m_Texture(texture), m_Width(m_Texture.getSize().x), m_Height(m_Texture.getSize().y)
{

}
