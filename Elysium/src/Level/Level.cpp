#include "Level.h"

#include "Asset/AssetManager.h"
#include "Texture.h"

Level::Level()
	: Level("Untitled")
{
}

Level::Level(const std::string& name)
	: m_Name(name)
{
}

Level::~Level()
{
}

void Level::OnUpdateEditor(sf::RenderTexture& renderTexture)
{
	// stuff
	RenderLevel(renderTexture);
}

void Level::RenderLevel(sf::RenderTexture& renderTexture)
{
	renderTexture.clear(sf::Color::Blue);

	for (auto& e : m_entityManager.getEntities())
	{
		/*if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.getSprite().setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
			e->getComponent<CAnimation>().animation.getSprite().setScale(e->getComponent<CTransform>().scale.x, e->getComponent<CTransform>().scale.y);
			e->getComponent<CAnimation>().animation.getSprite().setRotation(e->getComponent<CTransform>().angle);
			renderTexture.draw(e->getComponent<CAnimation>().animation.getSprite());
		}*/
		if (e->hasComponent<CSpriteRenderer>())
		{
			if (e->getComponent<CSpriteRenderer>().texture != 0)
			{
				// highly inefficient drawing
				sf::Texture tex = AssetManager::GetAsset<Texture>(e->getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
				sf::Sprite sprite = sf::Sprite(tex);
				sprite.setOrigin(tex.getSize().x/2, tex.getSize().y/2);
				sprite.setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
				renderTexture.draw(sprite);
			}
		}
	}

}
