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
	/*if (!m_Shader.loadFromFile("D:/Game Development/Game_Engine_Programming/Elysium/Sandbox Project/Assets/Shaders/vertex_shader.glsl", "D:/Game Development/Game_Engine_Programming/Elysium/Sandbox Project/Assets/Shaders/shader.glsl"))
	{
		std::cout << "couldnt load shader!\n";
	}*/
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
		if (e->hasComponent<CSpriteRenderer>())
		{
			if (e->getComponent<CSpriteRenderer>().texture != 0)
			{
				// highly inefficient drawing
				sf::Texture tex = AssetManager::GetAsset<Texture>(e->getComponent<CSpriteRenderer>().texture)->GetSFMLTexture();
				sf::Sprite sprite = sf::Sprite(tex);
				sprite.setOrigin(tex.getSize().x/2, tex.getSize().y/2);
				sprite.setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
				renderTexture.draw(sprite); // , & m_Shader);
			}
		}
	}
}
