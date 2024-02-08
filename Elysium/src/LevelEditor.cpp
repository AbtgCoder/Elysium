#include "GameEngine.h"
#include "LevelEditor.h"
#include "imgui.h"
#include "imgui-SFML.h"
#include <filesystem>
#include <cmath>
#include <fstream>

LevelEditor::LevelEditor(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}


void LevelEditor::init()
{
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	registerAction(sf::Keyboard::Escape, "QUIT");

	std::string assetDir = ".. /../../assets/textures/";
	loadAssets(assetDir);
}

void LevelEditor::loadLevel()
{

	m_entityManager = EntityManager();

	std::string filename = "level_test.txt";
	std::ifstream levelFile(filename);
	std::string entityType;
	while (levelFile >> entityType)
	{
		if (entityType == "tile")
		{
			std::string tileName;
			int gridX, gridY;
			levelFile >> tileName >> gridX >> gridY;
			auto tile = m_entityManager.addEntity("tile");
			tile->addComponent<CAnimation>(Animation(tileName, m_assets[tileName]), true);
			tile->addComponent<CTransform>(gridToMidPixel(gridX, gridY, tile));
			//tile->addComponent<CBoundingBox>(tile->getComponent<CAnimation>().animation.getSize());
		}
		if (entityType == "Dec")
		{
			std::string tileName;
			int gridX, gridY;
			levelFile >> tileName >> gridX >> gridY;
			auto tile = m_entityManager.addEntity("dec");
			tile->addComponent<CAnimation>(Animation(tileName, m_assets[tileName]), true);
			tile->addComponent<CTransform>(gridToMidPixel(gridX, gridY, tile));
		}
		if (entityType == "Player")
		{
			// TODO
		}
	}
	levelFile.close();
}

void LevelEditor::loadAssets(const std::string& path)
{
	std::string assetDir =  "D:/Game Development/comp4300_game_programming/Mega Mario/Assets/textures";

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

	m_entityManager = EntityManager();
}

void LevelEditor::saveLevel()
{
	std::ofstream outputFile("level_test.txt");

	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open file!" << std::endl;
	}

	for (auto e : m_entityManager.getEntities())
	{
		std::string line = "";
		line += e->tag();
		line += " ";
		line += e->getComponent<CAnimation>().animation.getName();
		line += " ";
		Vec2 ePos = worldToGrid(e);
		line += std::to_string((int)ePos.x);
		line += " ";
		line += std::to_string((int)ePos.y);
		line += "\n";
		outputFile << line;
	}

	outputFile.close();

}

Vec2 LevelEditor::worldToGrid(std::shared_ptr<Entity> entity)
{
	Vec2 animSize = entity->getComponent<CAnimation>().animation.getSize();
	Vec2 ePos = entity->getComponent<CTransform>().pos;
	float gridX = (ePos.x - (animSize.x / 2)) / m_gridSize.x;
	float gridY = (m_game->window().getSize().y - ePos.y - (animSize.y / 2)) / m_gridSize.y;
	return Vec2(gridX, gridY);
}

Vec2 LevelEditor::gridToMidPixel(float gridX, float gridY, std::shared_ptr<Entity> entity)
{
	Vec2 animSize = entity->getComponent<CAnimation>().animation.getSize();
	return Vec2(gridX * m_gridSize.x + (animSize.x / 2), m_game->window().getSize().y - (gridY * m_gridSize.y + (animSize.y / 2)));
}

Vec2 LevelEditor::gridToMidPixel(float gridX, float gridY)
{
	return Vec2(gridX * m_gridSize.x + (m_gridSize.x / 2), m_game->window().getSize().y - (gridY * m_gridSize.y + (m_gridSize.y / 2)));
}

void LevelEditor::snapToGrid(std::shared_ptr<Entity> entity)
{
	Vec2 ePos(entity->getComponent<CTransform>().pos.x - (entity->getComponent<CAnimation>().animation.getSize() / 2).x, entity->getComponent<CTransform>().pos.y + (entity->getComponent<CAnimation>().animation.getSize() / 2).y);
	int gridX = std::round(ePos.x / m_gridSize.x), gridY = std::round((m_game->window().getSize().y - ePos.y) / m_gridSize.y);
	entity->getComponent<CTransform>().pos = gridToMidPixel(gridX, gridY, entity);
}

void LevelEditor::spawnEntity(const std::string& name, const sf::Texture& tex)
{
	auto e = m_entityManager.addEntity("tile");
	e->addComponent<CAnimation>(Animation(name, tex), true);
	e->addComponent<CTransform>(m_mousePos);
	e->addComponent<CDraggable>(true);
}

Vec2 LevelEditor::windowToWorld(const Vec2& window) const
{
	auto view = m_game->window().getView();
	float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
	return Vec2(window.x + wx, window.y);
}


void LevelEditor::update()
{
	m_entityManager.update();
	sDrag();
	sGUI();
	sRender();
}

void LevelEditor::sDrag()
{
	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CDraggable>() && e->getComponent<CDraggable>().dragging)
		{
			Vec2 wPos = windowToWorld(m_mousePos);
			e->getComponent<CTransform>().pos = wPos;
		}
	}
}

void LevelEditor::sGUI()
{

	ImGui::Begin("Level Editor");

	if (ImGui::BeginTabBar("tab bar"))
	{
		if (ImGui::BeginTabItem("File"))
		{
			if (ImGui::Button("Load Assets"))
			{
				//
			}
			if (ImGui::Button("Load Level"))
			{
				loadLevel();
			}
			if (ImGui::Button("Save Level"))
			{
				saveLevel();
			}
			
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Assets"))
		{
			for (const auto& [name, texture] : m_assets)
			{
				if (ImGui::ImageButton(texture))
				{
					spawnEntity(name, texture);
				}
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void LevelEditor::sRender()
{
	sf::RenderWindow& window = m_game->window();
	window.clear(sf::Color(100, 100, 255));

	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.getSprite().setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
			window.draw(e->getComponent<CAnimation>().animation.getSprite());
		}
	}

	if (m_drawGrid)
	{
		for (int x = 0; x < 50; x++)
		{
			for (int y = 0; y < 12; y++)
			{
				sf::RectangleShape rect(sf::Vector2f(m_gridSize.x, m_gridSize.y));
				rect.setOrigin(m_gridSize.x / 2, m_gridSize.y / 2);
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
				text.setPosition(gridCellPos.x - (m_gridSize.x / 2) + 5, gridCellPos.y - (m_gridSize.y / 2) + 5);
				window.draw(text);
			}
		}
	}

	sf::CircleShape dot;
	dot.setFillColor(sf::Color::Red);
	dot.setRadius(8);
	dot.setOrigin(8, 8);
	Vec2 worldPos = windowToWorld(m_mousePos);
	dot.setPosition(worldPos.x, worldPos.y);
	window.draw(dot);
}

bool IsInside(Vec2 pos, std::shared_ptr<Entity> e)
{
	Vec2 s = e->getComponent<CAnimation>().animation.getSize();
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

void LevelEditor::sDoAction(const Action& action)
{
	if (action.name() == "MOUSE_MOVE")
	{
		m_mousePos = action.pos();
	}

	if (action.type() == "START")
	{
		if (action.name() == "TOGGLE_GRID")
		{
			m_drawGrid = !m_drawGrid;
		}
		else if (action.name() == "LEFT_CLICK")
		{
			// detect the picking up of entities
			Vec2 wPos = windowToWorld(m_mousePos);
			for (auto e : m_entityManager.getEntities())
			{
				if (IsInside(wPos, e))
				{
					if (!e->hasComponent<CDraggable>()) { continue; }

					auto& dragging = e->getComponent<CDraggable>().dragging;
					
					if (!dragging)
					{
						dragging = true;
					}
					else
					{
						dragging = false;
						snapToGrid(e);
					}
					break;
				}
			}
		}
		else if (action.name() == "QUIT")
		{
			m_hasEnded = true;
			onEnd();
		}
	}
	
}


void LevelEditor::onEnd()
{
	m_game->quit();

}