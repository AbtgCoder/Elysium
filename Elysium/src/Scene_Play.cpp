#include "Scene_Play.h"
#include "Scene_Menu.h"
#include "LevelEditor.h"
#include "Physics.h"
#include "Assets.h"
#include "GameEngine.h"
#include "Components.h"
#include "Action.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <yaml-cpp/yaml.h>

#include <iostream>
#include <fstream>

namespace YAML
{
	template<>
	struct convert<Vec2>
	{
		static Node encode(const Vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};
}



Scene_Play::Scene_Play(GameEngine* gameEngine, std::string& levelPath)
	: Scene(gameEngine), m_levelPath(levelPath)
{
	init(m_levelPath);
}

void Scene_Play::init(const std::string& levelPath)
{
	registerAction(sf::Keyboard::P, "PAUSE");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::T, "TOGGLE_TEXTURE");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	registerAction(sf::Keyboard::A, "LEFT");
	registerAction(sf::Keyboard::D, "RIGHT");
	registerAction(sf::Keyboard::Space, "JUMP");

	m_gameView = m_game->window().getDefaultView();

	m_gridText.setCharacterSize(20);
	m_gridText.setFont(m_game->assets().getFont("Tech"));
	m_gridText.setPosition(sf::Vector2f(5, 5));

	std::string assetDir = "../../../Assets/textures/";
	loadAssets(assetDir);
	loadLevel(levelPath);
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	Vec2 animSize = entity->getComponent<CAnimation>().animation.getSize();
	return Vec2(gridX * m_gridSize.x + (animSize.x / 2), m_game->window().getSize().y - (gridY * m_gridSize.y + (animSize.y / 2)));
}

Vec2 Scene_Play::gridToMidPixel(float gridX, float gridY)
{
	return Vec2(gridX*m_gridSize.x + (m_gridSize.x/2), m_game->window().getSize().y - (gridY * m_gridSize.y + (m_gridSize.y / 2)));
}

void Scene_Play::spawnPlayer()
{
	for (auto entity : m_entityManager.getEntities())
	{
		if (entity->tag() == "Player")
		{
			entity->addComponent<CState>("air");
			entity->addComponent<CInput>();
			m_player = entity;
			break;
		}
	}

}

void Scene_Play::loadAssets(const std::string& assetDir)
{
	for (const auto& entry : std::filesystem::directory_iterator(assetDir))
	{
		if (!entry.is_directory())
		{
			std::string fileName = entry.path().string();
			sf::Texture texture;
			if (texture.loadFromFile(fileName))
			{
				m_assets[entry.path().stem().string()] = texture;
			}
			else
			{
				std::cerr << "Failed to load texture from file: " << fileName << std::endl;
			}
		}
	}
}

bool Scene_Play::loadLevel(const std::filesystem::path& filepath)
{
	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "Couldnt load file: " << filepath << "\n";
		return false;
	}

	if (!data["Level"])
	{
		std::cout << "no level data\n";
		return false;
	}

	m_entityManager = EntityManager();

	std::string levelName = data["Level"].as<std::string>();

	auto entities = data["Entities"];
	if (entities)
	{
		for (auto entity : entities)
		{
			auto tag = entity["Entity"].as<std::string>();
			auto deserializedEntity = m_entityManager.addEntity(tag);
			deserializedEntity->addComponent<CTag>(tag);
			
			auto transformComponent = entity["Transform"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity->addComponent<CTransform>();
				tc.pos = transformComponent["Position"].as<Vec2>();
				tc.prevPos = tc.pos;
				tc.scale = transformComponent["Scale"].as<Vec2>();
				tc.angle = transformComponent["Angle"].as<float>();
			}

			auto animationComponent = entity["Animation"];
			if (animationComponent)
			{
				std::string textureName = animationComponent["Texture"].as<std::string>();
				size_t animationSpeed = animationComponent["Speed"].as<size_t>();
				size_t frameCount = animationComponent["Frames"].as<size_t>();
				bool repeat = animationComponent["Repeatable"].as<bool>();
				auto& ac = deserializedEntity->addComponent<CAnimation>(Animation(textureName, m_assets[textureName], frameCount, animationSpeed), repeat);
				ac.animSpeed = animationSpeed;
				ac.frameCount = frameCount;
				ac.layer = animationComponent["Layer"].as<int>();
			}

			auto boundingBoxComponent = entity["AABB"];
			if (boundingBoxComponent)
			{
				auto& bc2d = deserializedEntity->addComponent<CBoundingBox>();
				bc2d.offset = boundingBoxComponent["Offset"].as<Vec2>();
				bc2d.size = boundingBoxComponent["Size"].as<Vec2>();
				bc2d.halfSize = bc2d.size / 2;
			}

			auto polygonColliderComponent = entity["PolygonCollider"];
			if (polygonColliderComponent)
			{
				auto& pc2d = deserializedEntity->addComponent<CPolygonCollider>();
				pc2d.offset = polygonColliderComponent["Offset"].as<Vec2>();
				auto verticesData = polygonColliderComponent["Points"];
				auto elements = verticesData["Elements"];
				for (auto element : elements)
				{
					pc2d.colliderVertices.push_back(element.as<Vec2>());
				}
			}

			auto gravityComponent = entity["Gravity"];
			if (gravityComponent)
			{
				auto& gc = deserializedEntity->addComponent<CGravity>();
				gc.gravity = gravityComponent["Gravity"].as<float>();
			}
		}
	}

	m_entityManager.update();
	spawnPlayer();
	return true;
}

void Scene_Play::update()
{
	m_currentFrame++;
	m_entityManager.update();
	//sEnemySpawner();
	sMovement();
	sCollision();
	sAnimation();
	sCamera();
	sRender();
}

void Scene_Play::sAnimation()
{
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			if (!e->getComponent<CAnimation>().repeat && e->getComponent<CAnimation>().animation.hasEnded())
			{
				e->destroy();
			}
			if (e->getComponent<CAnimation>().animation.getSpeed() != 0)
			{
				e->getComponent<CAnimation>().animation.update();
			}
		}
	}

}

void Scene_Play::sMovement()
{
	// Check for left/right movement and apply acceleration
	if (m_player->getComponent<CState>().state != "air") 
	{
		if (m_player->getComponent<CInput>().left) 
		{
			if (m_player->getComponent<CState>().state != "running_left")
			{
				m_player->getComponent<CState>().state = "running_left";
				//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("runleft"), true);
			}
			m_player->getComponent<CTransform>().velocity.x -= 5.0f;
			if (m_player->getComponent<CTransform>().velocity.x < -5.0f)
			{
				m_player->getComponent<CTransform>().velocity.x = -5.0f;
			}
		}
		else if (m_player->getComponent<CInput>().right)
		{
			if (m_player->getComponent<CState>().state != "running_right")
			{
				m_player->getComponent<CState>().state = "running_right";
				//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("runright"), true);
			}
			m_player->getComponent<CTransform>().velocity.x += 5.0f;
			if (m_player->getComponent<CTransform>().velocity.x > 5.0f)
			{
				m_player->getComponent<CTransform>().velocity.x = 5.0f;
			}
		}
		else
		{
			m_player->getComponent<CTransform>().velocity.x = 0;
			if (m_player->getComponent<CState>().state != "ground")
			{
				m_player->getComponent<CState>().state = "ground";
				//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("player"), true);
			}
		}
	}


	// Apply gravity
	m_player->getComponent<CTransform>().velocity.y +=  m_player->getComponent<CGravity>().gravity;


	// Check for jump
	if (m_player->getComponent<CState>().state != "air" && m_player->getComponent<CInput>().up) 
	{
		m_player->getComponent<CTransform>().velocity.y = -20.0f;
		m_player->getComponent<CState>().state = "air";
		//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("air"), true);
	}

	if (m_player->getComponent<CTransform>().velocity.y > 20)
	{
		m_player->getComponent<CTransform>().velocity.y = 20.0f;
	}


	// Apply velocity
	m_player->getComponent<CTransform>().prevPos = m_player->getComponent<CTransform>().pos;
	m_player->getComponent<CTransform>().pos += m_player->getComponent<CTransform>().velocity;

	if (m_player->getComponent<CTransform>().pos.x - m_player->getComponent<CBoundingBox>().size.x / 2 < m_gameView.getCenter().x - m_game->window().getSize().x/2)
	{
		m_player->getComponent<CTransform>().pos.x = m_gameView.getCenter().x - m_game->window().getSize().x / 2 + m_player->getComponent<CBoundingBox>().size.x / 2;
	}

	if (m_player->getComponent<CTransform>().pos.y > m_game->window().getSize().y)
	{
		m_player->destroy();
		spawnPlayer();
	}

	for (auto& enemy : m_entityManager.getEntities("enemy"))
	{
		enemy->getComponent<CTransform>().velocity.y += enemy->getComponent<CGravity>().gravity;
		enemy->getComponent<CTransform>().prevPos = enemy->getComponent<CTransform>().pos;
		enemy->getComponent<CTransform>().pos += enemy->getComponent<CTransform>().velocity;
		if (enemy->getComponent<CTransform>().pos.x < m_gameView.getCenter().x - 400 - m_game->window().getSize().x / 2)
		{
			enemy->destroy();
		}
	}

}

void Scene_Play::sEnemySpawner()
{
	if (m_currentFrame - m_lastEnemySpawnTime >= m_enemySpawnInterval && m_numEnemiesSpawned < m_numEnemiesAllowed)
	{
		auto enemy = m_entityManager.addEntity("enemy");
		enemy->addComponent<CAnimation>(m_game->assets().getAnimation("enemy"), true);
		enemy->addComponent<CTransform>(gridToMidPixel(15, 6), Vec2(-4.0f, 0.0f), Vec2(1.0f, 1.0f), 0.0f);
		enemy->addComponent<CBoundingBox>(enemy->getComponent<CAnimation>().animation.getSize());
		enemy->addComponent<CGravity>(m_playerConfig.GRAVITY);
		enemy->addComponent<CScore>(20);
		m_lastEnemySpawnTime = m_currentFrame;
		m_numEnemiesSpawned++;
	}
}

void Scene_Play::sCollision()
{
	for (auto& e : m_entityManager.getEntities("Tile"))
	{
		Vec2 overlap = Physics::GetOverlap(m_player, e);
		if (overlap.x > 0 && overlap.y > 0)
		{
			if (e->getComponent<CAnimation>().animation.getName() == "coin")
			{
				e->destroy();
				m_score += e->getComponent<CScore>().score;
			}
			else
			{
				Vec2 prevOverlap = Physics::GetPreviousOverlap(m_player, e);
				if (prevOverlap.y > 0)
				{
					// horizontal
					if (m_player->getComponent<CTransform>().prevPos.x < m_player->getComponent<CTransform>().pos.x)
					{
						// from left
						m_player->getComponent<CTransform>().prevPos.x = m_player->getComponent<CTransform>().pos.x;
						m_player->getComponent<CTransform>().pos.x -= overlap.x;
					}
					else
					{
						// from right
						m_player->getComponent<CTransform>().prevPos.x = m_player->getComponent<CTransform>().pos.x;
						m_player->getComponent<CTransform>().pos.x += overlap.x;
					}
				}
				if (prevOverlap.x > 0)
				{

					// vertical
					if (m_player->getComponent<CTransform>().prevPos.y < m_player->getComponent<CTransform>().pos.y)
					{
						// from top
						m_player->getComponent<CTransform>().prevPos.y = m_player->getComponent<CTransform>().pos.y;
						m_player->getComponent<CTransform>().pos.y -= overlap.y;
						m_player->getComponent<CTransform>().velocity.y = 0.0f;
						if (m_player->getComponent<CState>().state == "air")
						{
							m_player->getComponent<CState>().state = "ground";
							//m_player->addComponent<CAnimation>(m_game->assets().getAnimation("player"), true);
						}
					}
					else
					{
						// from bottom
						m_player->getComponent<CTransform>().prevPos.y = m_player->getComponent<CTransform>().pos.y;
						m_player->getComponent<CTransform>().pos.y += overlap.y;
						m_player->getComponent<CTransform>().velocity.y = 0.0f;
						if (overlap.x > e->getComponent<CAnimation>().animation.getSize().y / 2)
						{
							if (e->getComponent<CAnimation>().animation.getName() == "brick")
							{
								e->destroy();
							}
							else if (e->getComponent<CAnimation>().animation.getName() == "question")
							{
								e->destroy();
								auto coin = m_entityManager.addEntity("tile");
								coin->addComponent<CAnimation>(m_game->assets().getAnimation("coin"), true);
								coin->addComponent<CTransform>(e->getComponent<CTransform>().pos - Vec2(0, m_gridSize.y));
								coin->addComponent<CBoundingBox>(coin->getComponent<CAnimation>().animation.getSize());
								coin->addComponent<CScore>(10);
							}
						}
					
					}
				}
			}

		}
	
		for (auto& enemy : m_entityManager.getEntities("enemy"))
		{
			Vec2 overlap = Physics::GetOverlap(enemy, e);
			if (overlap.x > 0 && overlap.y > 0)
			{
				Vec2 prevOverlap = Physics::GetPreviousOverlap(enemy, e);
				if (prevOverlap.y > 0)
				{
					// horizontal
					enemy->getComponent<CTransform>().velocity.x *= -1;
					enemy->getComponent<CTransform>().prevPos.x = enemy->getComponent<CTransform>().pos.x;
					if (enemy->getComponent<CTransform>().prevPos.x > enemy->getComponent<CTransform>().pos.x)
					{
						// from right
						enemy->getComponent<CTransform>().pos.x += overlap.x;
					}
					else
					{
						// from left
						enemy->getComponent<CTransform>().pos.x -= overlap.x;
					}
				}
				if (prevOverlap.x > 0)
				{
					// vertical
					if (enemy->getComponent<CTransform>().prevPos.y < enemy->getComponent<CTransform>().pos.y)
					{
						// from top
						enemy->getComponent<CTransform>().prevPos.y = enemy->getComponent<CTransform>().pos.y;
						enemy->getComponent<CTransform>().pos.y -= overlap.y;
						enemy->getComponent<CTransform>().velocity.y = 0.0f;
					}
				}

			}
		}
	}


}

void Scene_Play::sCamera()
{
	float shiftX = m_player->getComponent<CTransform>().pos.x - m_gameView.getCenter().x;
	float interpolationFactor = 0.01f; // viewMoveSpeed * deltaTime
	if (shiftX > 0.2 * m_game->window().getSize().x)
	{
		sf::Vector2f targetPosition = m_gameView.getCenter() + sf::Vector2f(shiftX, 0);
		m_gameView.setCenter(m_gameView.getCenter() + interpolationFactor * (targetPosition - m_gameView.getCenter()));
	}
}

void Scene_Play::sRender()
{
	sf::RenderWindow& window = m_game->window();
	window.setView(m_gameView);
	window.clear(sf::Color(100, 100, 255));
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.getSprite().setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
			if (m_drawTextures) 
			{
				window.draw(e->getComponent<CAnimation>().animation.getSprite());
			}
		}
		if (m_drawCollision)
		{
			if (e->hasComponent<CBoundingBox>())
			{
				Vec2 rectSize = e->getComponent<CBoundingBox>().size;
				sf::RectangleShape rect(sf::Vector2f(rectSize.x, rectSize.y));
				rect.setOrigin(rectSize.x/2,rectSize.y/2);
				rect.setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
				rect.setFillColor(sf::Color::Transparent);
				rect.setOutlineColor(sf::Color::White);
				rect.setOutlineThickness(1);
				window.draw(rect);
			}
		}
	}

	if (m_drawGrid)
	{
		for (int x=0; x < 50; x++)
		{
			for (int y = 0; y < 12; y++)
			{
				sf::RectangleShape rect(sf::Vector2f(m_gridSize.x, m_gridSize.y));
				rect.setOrigin(m_gridSize.x/2, m_gridSize.y/2);
				Vec2 gridCellPos = gridToMidPixel(x, y);
				rect.setPosition(gridCellPos.x, gridCellPos.y);
				rect.setFillColor(sf::Color::Transparent);
				rect.setOutlineColor(sf::Color::White);
				rect.setOutlineThickness(1);
				window.draw(rect);
				sf::Text text;
				text.setFont(m_game->assets().getFont("Tech"));
				text.setString("(" + std::to_string(x) + "," + std::to_string(y) + ")");
				text.setCharacterSize(12);
				text.setPosition(gridCellPos.x - (m_gridSize.x/2) + 5, gridCellPos.y - (m_gridSize.y / 2) + 5);
				window.draw(text);
			}
		}
	}
	m_gridText.setString("Score: " + std::to_string(m_score));
	m_gridText.setPosition(m_gameView.getCenter() - 0.5f*m_gameView.getSize() + sf::Vector2f(5, 5));
	window.draw(m_gridText);
	//window.display();
}

void Scene_Play::sDoAction(const Action& action)
{
	if (action.type() == "START")
	{
		if (action.name() == "TOGGLE_TEXTURE")
		{
			m_drawTextures = !m_drawTextures;
		}
		else if (action.name() == "TOGGLE_COLLISION")
		{
			m_drawCollision = !m_drawCollision;
		}
		else if (action.name() == "TOGGLE_GRID")
		{
			m_drawGrid = !m_drawGrid;
		}
		else if (action.name() == "PAUSE")
		{
			m_paused = !m_paused;
		}
		else if (action.name() == "QUIT")
		{
			onEnd();
		}
		else if (action.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = true;
		}
		else if (action.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = true;
		}
		else if (action.name() == "JUMP")
		{
			m_player->getComponent<CInput>().up = true;
		}
	}

	if (action.type() == "END")
	{
		if (action.name() == "RIGHT")
		{
			m_player->getComponent<CInput>().right = false;
		}
		else if (action.name() == "LEFT")
		{
			m_player->getComponent<CInput>().left = false;
		}
		else if (action.name() == "JUMP")
		{
			m_player->getComponent<CInput>().up = false;
		}
	}

}

void Scene_Play::onEnd()
{

	// change back to menu scene
	for (auto& e : m_entityManager.getEntities())
	{
		e->destroy();
	}
	m_entityManager.update();
	m_hasEnded = true;
	m_game->window().setView(m_game->window().getDefaultView());
	std::shared_ptr<Scene> levelEditor = std::make_shared<LevelEditor>(m_game);
	m_game->changeScene("level_editor", levelEditor, m_hasEnded);
}
