#include "Level.h"

#include "Asset/AssetManager.h"
#include "Core/Texture.h"

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

std::shared_ptr<Entity> Level::AddEntity(std::shared_ptr<Entity> entity)
{
	return m_entityManager.addEntity(entity);
}

std::shared_ptr<Entity> Level::AddEntityWithSprite(Vec2 pos, AssetHandle textureHandle)
{
	// asset, asset type as texture
	auto entity = m_entityManager.addEntity("Tile");
	entity->addComponent<CTag>("Tile");
	entity->addComponent<CTransform>(pos);
	entity->addComponent<CSpriteRenderer>();
	entity->getComponent<CSpriteRenderer>().texture = textureHandle;
	return entity;
}

static bool IsInside(Vec2 pos, std::shared_ptr<Entity> e)
{
	if (e->hasComponent<CSpriteRenderer>())
	{
		sf::Vector2u s =  AssetManager::GetAsset<Texture>(e->getComponent<CSpriteRenderer>().texture)->GetSFMLTexture().getSize();
		Vec2 ePos = e->getComponent<CTransform>().pos;
		if (pos.x > ePos.x - s.x / 2 &&
			pos.x < ePos.x + s.x / 2 &&
			pos.y > ePos.y - s.y / 2 &&
			pos.y < ePos.y + s.y / 2)
		{
			return true;
		}
		return false;
	}
	
	return false;
}

std::shared_ptr<Entity> Level::GetEntityIfClicked(Vec2 mousePos)
{
	for (auto entity : m_entityManager.getEntities())
	{
		if (IsInside(mousePos, entity))
		{
			return entity;
		}
	}

	return nullptr;
}

void Level::DestroyEntity(std::shared_ptr<Entity> entity)
{
	entity->destroy();
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
		if (e->hasComponent<CSpriteRenderer>())
		{
			if (e->getComponent<CSpriteRenderer>().texture != 0)
			{
				// highly inefficient drawing
				sf::Texture tex = AssetManager::GetAsset<Texture>(e->getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
				sf::Sprite sprite = sf::Sprite(tex);
				sprite.setOrigin(tex.getSize().x/2, tex.getSize().y/2);
				sprite.setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
				//renderTexture.draw(sprite, &m_Shader);
				renderTexture.draw(sprite);
			}
		}
	}
}
