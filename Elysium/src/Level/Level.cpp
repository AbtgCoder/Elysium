#include "Level.h"

#include "Asset/AssetManager.h"
#include "Core/Texture.h"
#include "Physics/Physics.h"

Level::Level()
	: Level("Untitled")
{
}

Level::Level(const std::string& name)
	: m_Name(name)
{
	m_PhysicsRect.setFillColor(sf::Color::Transparent);
	m_PhysicsRect.setOutlineColor(sf::Color::White);
	m_PhysicsRect.setOutlineThickness(1);

	m_PhysicsPoly.setFillColor(sf::Color::Transparent);
	m_PhysicsPoly.setOutlineColor(sf::Color::White);
	m_PhysicsPoly.setOutlineThickness(1);
}

Level::~Level()
{
}

Entity Level::AddEntity(Entity entity)
{
	return {};// m_entityManager.addEntity(entity);
}

Entity Level::AddEntityWithSprite(Vec2 pos, AssetHandle textureHandle)
{
	// asset, asset type as texture
	auto entity = m_entityManager.addEntity();
	entity.addComponent<CTag>("Tile");
	entity.addComponent<CTransform>(pos);
	entity.addComponent<CSpriteRenderer>();
	entity.getComponent<CSpriteRenderer>().texture = textureHandle;
	return entity;
}

static bool IsInside(Vec2 pos, Entity e)
{
	if (e.hasComponent<CSpriteRenderer>())
	{
		sf::Vector2u s =  AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture().getSize();
		Vec2 ePos = e.getComponent<CTransform>().pos;
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

Entity Level::GetEntityIfClicked(Vec2 mousePos)
{
	for (auto entity : m_entityManager.GetEntities())
	{
		if (IsInside(mousePos, entity))
		{
			return entity;
		}
	}

	return {};
}

void Level::DestroyEntity(Entity entity)
{
	entity.destroy();
}

std::vector<Entity>& Level::GetAllPhysicsEntities()
{
	std::vector<Entity> physicsEntities;
	for (auto e : m_entityManager.GetEntities())
	{
		if (e.hasComponent<CBoundingBox>())
		{
			physicsEntities.push_back(e);
		}
	}
	return physicsEntities;
}

void Level::OnUpdateRuntime(sf::RenderTexture& renderTexture, bool drawPhysicsColliders)
{
	if (!m_IsPaused)
	{
		// Physics
		for (auto e_i : m_entityManager.GetEntities())
		{
			e_i.getComponent<CTransform>().prevPos = e_i.getComponent<CTransform>().pos;
			if (e_i.getComponent<CTransform>().pos.y > 750 || e_i.getComponent<CTransform>().pos.y < 0)
			{
				e_i.getComponent<CTransform>().velocity.y *= -1;
			}
			if (e_i.getComponent<CTransform>().pos.x > 1000 || e_i.getComponent<CTransform>().pos.x < -250)
			{
				e_i.getComponent<CTransform>().velocity.x *= -1;
			}
			e_i.getComponent<CTransform>().pos += e_i.getComponent<CTransform>().velocity;

			for (auto e_j : m_entityManager.GetEntities())
			{
				if (e_i.hasComponent<CPolygonCollider>(), e_j.hasComponent<CPolygonCollider>())
				{
					if (Physics::SAT(e_i, e_j))
					{
						std::cout << e_i.getComponent<CTag>().tag << " collided with " << e_j.getComponent<CTag>().tag << "\n";
					}
				}
			}
		}
	}


	// Rendering
	RenderLevel(renderTexture, drawPhysicsColliders);
}

void Level::OnUpdateEditor(sf::RenderTexture& renderTexture, bool drawPhysicsColliders)
{
	// stuff
	RenderLevel(renderTexture, drawPhysicsColliders);
}

void Level::RenderLevel(sf::RenderTexture& renderTexture, bool drawPhysicsColliders)
{
	renderTexture.clear(sf::Color::Blue);
	for (auto e : m_entityManager.GetEntities())
	{
		//if (e.hasComponent<CSpriteRenderer>())
		//{
		//	if (e.getComponent<CSpriteRenderer>().texture != 0)
		//	{
		//		// highly inefficient drawing
		//		sf::Texture tex = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
		//		sf::Sprite sprite = sf::Sprite(tex);
		//		sprite.setOrigin(tex.getSize().x / 2, tex.getSize().y / 2);
		//		sprite.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
		//		//renderTexture.draw(sprite, &m_Shader);
		//		renderTexture.draw(sprite);
		//	}
		//}
		if (drawPhysicsColliders)
		{
			if (e.hasComponent<CBoundingBox>())
			{
				Vec2 rectSize = e.getComponent<CBoundingBox>().size;
				m_PhysicsRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
				m_PhysicsRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
				m_PhysicsRect.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
				renderTexture.draw(m_PhysicsRect);
			}
			if (e.hasComponent<CPolygonCollider>())
			{
				std::vector<Vec2> vertices = e.getComponent<CPolygonCollider>().colliderVertices;
				m_PhysicsPoly.setPointCount(vertices.size());
				Vec2 ePos = e.getComponent<CTransform>().pos;
				sf::Texture tex = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
				Vec2 eSize(tex.getSize().x, tex.getSize().y);
				for (size_t i = 0; i < vertices.size(); i++)
				{
					m_PhysicsPoly.setPoint(i, sf::Vector2f(ePos.x - eSize.x/2 + vertices[i].x, ePos.y + eSize.y/2 - vertices[i].y));
				}
				renderTexture.draw(m_PhysicsPoly);
			}
		}
		else
		{
			if (e.hasComponent<CSpriteRenderer>())
			{
				if (e.getComponent<CSpriteRenderer>().texture != 0)
				{
					// highly inefficient drawing
					sf::Texture tex = AssetManager::GetAsset<Texture>(e.getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
					sf::Sprite sprite = sf::Sprite(tex);
					sprite.setOrigin(tex.getSize().x / 2, tex.getSize().y / 2);
					sprite.setPosition(e.getComponent<CTransform>().pos.x, e.getComponent<CTransform>().pos.y);
					//renderTexture.draw(sprite, &m_Shader);
					renderTexture.draw(sprite);
				}
			}
		}

	}
}
