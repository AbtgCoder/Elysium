#pragma once

#include "Asset/Asset.h"

#include <SFML/Graphics.hpp>

class Texture : public Asset
{
public:
	Texture() = default;
	Texture(const sf::Texture& texture);
	virtual AssetType GetType() const { return AssetType::Texture; }

	sf::Texture& GetSFMLTexture() { return m_Texture; }
private:
	sf::Texture m_Texture;
	uint32_t m_Width, m_Height;
};