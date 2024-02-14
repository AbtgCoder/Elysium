#include "GameEngine.h"
#include "LevelEditor.h"
#include "Physics.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <filesystem>
#include <cmath>
#include <fstream>
#include <algorithm>

LevelEditor::LevelEditor(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}


void LevelEditor::init()
{
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Delete, "DELETE");
	registerAction(sf::Keyboard::D, "DUPLICATE");

	// Set ImGui Styles
	setImGuiStyle();

	m_gameView.reset(sf::FloatRect(0, 0, 1264, 762));
	m_gameView.setViewport(sf::FloatRect(0, 0, 0.8, 1));

	m_gridRect.setSize(sf::Vector2f(m_gridSize.x, m_gridSize.y));
	m_gridRect.setOrigin(m_gridSize.x / 2, m_gridSize.y / 2);
	m_gridRect.setFillColor(sf::Color::Transparent);
	m_gridRect.setOutlineColor(sf::Color::White);
	m_gridRect.setOutlineThickness(1);
	m_gridText.setFont(m_game->assets().getFont("Tech"));
	m_gridText.setCharacterSize(10);

	m_collisionRect.setFillColor(sf::Color::Transparent);
	m_collisionRect.setOutlineColor(sf::Color::White);
	m_collisionRect.setOutlineThickness(1);

	m_cursorDot.setFillColor(sf::Color::Red);
	m_cursorDot.setRadius(8);
	m_cursorDot.setOrigin(8, 8);

	m_gameBG.setSize(sf::Vector2f(m_gameView.getSize().x, m_gameView.getSize().y-2));
	m_gameBG.setFillColor(sf::Color(100, 100, 255));
	m_gameBG.setOutlineThickness(1);
	m_gameBG.setOutlineColor(sf::Color(45, 45, 45));
	m_gameBG.setPosition(sf::Vector2f(1.f, 1.f));

	std::string assetDir = "../../../Assets/textures/";
	loadAssets(assetDir);
	loadLevel();
}

void LevelEditor::setImGuiStyle()
{


	auto& colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_WindowBg] = ImVec4{ 0.08f, 0.08f, 0.08f, 1.0f };
	//colors[ImGuiCol_WindowBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 1.0f };
	colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Border
	colors[ImGuiCol_Border] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_BorderShadow] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.24f };

	// Text
	colors[ImGuiCol_Text] = ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
	colors[ImGuiCol_TextDisabled] = ImVec4{ 0.5f, 0.5f, 0.5f, 1.0f };

	// Headers
	colors[ImGuiCol_Header] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
	colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_HeaderActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Buttons
	colors[ImGuiCol_Button] = ImVec4{ 0.13f, 0.13f, 0.17, 1.0f };
	colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ButtonActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_CheckMark] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };

	// Popups
	colors[ImGuiCol_PopupBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 0.92f };

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.54f };
	colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.54f };

	// Frame BG
	colors[ImGuiCol_FrameBg] = ImVec4{ 0.13f, 0.13, 0.17, 1.0f };
	colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Tabs
	colors[ImGuiCol_Tab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabHovered] = ImVec4{ 0.24, 0.24f, 0.32f, 1.0f };
	colors[ImGuiCol_TabActive] = ImVec4{ 0.2f, 0.22f, 0.27f, 1.0f };
	colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };

	// Scrollbar
	colors[ImGuiCol_ScrollbarBg] = ImVec4{ 0.1f, 0.1f, 0.13f, 1.0f };
	colors[ImGuiCol_ScrollbarGrab] = ImVec4{ 0.16f, 0.16f, 0.21f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4{ 0.19f, 0.2f, 0.25f, 1.0f };
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4{ 0.24f, 0.24f, 0.32f, 1.0f };

	// Seperator
	colors[ImGuiCol_Separator] = ImVec4{ 0.44f, 0.37f, 0.61f, 1.0f };
	colors[ImGuiCol_SeparatorHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 1.0f };
	colors[ImGuiCol_SeparatorActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 1.0f };

	// Resize Grip
	colors[ImGuiCol_ResizeGrip] = ImVec4{ 0.44f, 0.37f, 0.61f, 0.29f };
	colors[ImGuiCol_ResizeGripHovered] = ImVec4{ 0.74f, 0.58f, 0.98f, 0.29f };
	colors[ImGuiCol_ResizeGripActive] = ImVec4{ 0.84f, 0.58f, 1.0f, 0.29f };


	auto& style = ImGui::GetStyle();
	style.TabRounding = 4;
	style.ScrollbarRounding = 9;
	//style.WindowRounding = 7;
	style.GrabRounding = 3;
	style.FrameRounding = 3;
	style.PopupRounding = 4;
	style.ChildRounding = 4;


}

void LevelEditor::loadLevel()
{
	m_entityManager = EntityManager();

	std::string filename = "../../../Assets/levels/level_test2.txt";
	std::ifstream levelFile(filename);
	std::string entityType;
	while (levelFile >> entityType)
	{
		if (entityType == "Tile")
		{
			std::string tileName;
			int gridX, gridY;
			levelFile >> tileName >> gridX >> gridY;
			auto tile = m_entityManager.addEntity("Tile");
			tile->addComponent<CAnimation>(Animation(tileName, m_assets[tileName]), true);
			tile->addComponent<CTransform>(gridToMidPixel(gridX, gridY, tile));
			tile->addComponent<CDraggable>(false);
		}
		if (entityType == "Dec")
		{
			std::string tileName;
			int gridX, gridY;
			levelFile >> tileName >> gridX >> gridY;
			auto tile = m_entityManager.addEntity("Dec");
			tile->addComponent<CAnimation>(Animation(tileName, m_assets[tileName]), true);
			tile->addComponent<CTransform>(gridToMidPixel(gridX, gridY, tile));
			tile->addComponent<CDraggable>(false);
		}
		if (entityType == "Player")
		{
			// TODO
		}
	}
	levelFile.close();
}

void LevelEditor::loadAssets(const std::string& assetDir)
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

void LevelEditor::saveLevel()
{
	std::ofstream outputFile("../../../Assets/levels/level_test.txt");

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

Vec2 LevelEditor::windowToWorld(const Vec2& windowPos) const
{
	auto view = m_game->window().getView();
	float wx = view.getCenter().x - (m_game->window().getSize().x / 2);
	float wy = view.getCenter().y - (m_game->window().getSize().y / 2);
	return Vec2(windowPos.x + wx, windowPos.y + wy);
}

void LevelEditor::spawnEntity(const std::string& name, const sf::Texture& tex)
{
	auto e = m_entityManager.addEntity("Tile");
	e->addComponent<CAnimation>(Animation(name, tex), true, 0);
	e->addComponent<CTransform>(m_mousePos);
	e->addComponent<CDraggable>(true);
	m_inspectedEntity = e;
}

void LevelEditor::update()
{
	m_entityManager.update();
	if (m_enableDragging)
	{
		sDrag();
	}
	if (m_playAnimation)
	{
		sAnimation();
	}
	if (m_inspectedEntity)
	{
		//sCollision();
	}
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

void LevelEditor::sAnimation()
{
	m_inspectedEntity->getComponent<CAnimation>().animation.update();
}

void LevelEditor::sCollision()
{
	for (auto& e : m_entityManager.getEntities())
	{
		Vec2 overlap = Physics::GetOverlap(m_inspectedEntity, e);

		if (overlap.x > 0 && overlap.y > 0)
		{
		}
	}
}

void LevelEditor::entityInspectorGUI()
{
	Vec2 eWorldPos = m_inspectedEntity->getComponent<CTransform>().pos;
	Vec2 scale = m_inspectedEntity->getComponent<CTransform>().scale;
	float angle = m_inspectedEntity->getComponent<CTransform>().angle;
	Vec2 eGridPos = worldToGrid(m_inspectedEntity);
	int gridX = (int)eGridPos.x, gridY = (int)eGridPos.y;
	int animSpeed = (int)m_inspectedEntity->getComponent<CAnimation>().animation.getSpeed();
	int frameCount = (int)m_inspectedEntity->getComponent<CAnimation>().animation.getFrameCount();
	bool boundingBox = m_inspectedEntity->hasComponent<CBoundingBox>();
	if (ImGui::CollapsingHeader("Transform"))
	{
		ImGui::Columns(2, "", false);
		ImGui::Text("Grid X: ");
		ImGui::SameLine();
		ImGui::SliderInt("##Slider1", &gridX, 0, 20);
		ImGui::NextColumn();
		ImGui::Text("Grid Y: ");
		ImGui::SameLine();
		ImGui::SliderInt("##Slider2", &gridY, 0, 20);
		ImGui::NextColumn();
		ImGui::Text("Scale X: ");
		ImGui::SameLine();
		ImGui::SliderFloat("##Slider3", &scale.x, 0, 20);
		ImGui::NextColumn();
		ImGui::Text("Scale Y: ");
		ImGui::SameLine();
		ImGui::SliderFloat("##Slider4", &scale.y, 0, 20);
		ImGui::Columns(1);
		ImGui::Text("Angle: ");
		ImGui::SameLine();
		ImGui::SliderFloat("##Slider5", &angle, 0, 360);
	}
	if (ImGui::CollapsingHeader("Animation"))
	{
		ImGui::Text("Sprite: ");
		ImGui::SameLine();
		ImGui::Image(m_inspectedEntity->getComponent<CAnimation>().animation.getSprite(), sf::Vector2f(64, 64));
		ImGui::Text("Animation Speed: ");
		ImGui::SameLine();
		ImGui::SliderInt("##Slider6", &animSpeed, 0, 120);
		ImGui::Text("Num of Animation Frames: ");
		ImGui::SameLine();
		ImGui::SliderInt("##Slider7", &frameCount, 1, 20);
		ImGui::Checkbox("Repeatable", &m_inspectedEntity->getComponent<CAnimation>().repeat);
		ImGui::SliderInt("Layer", &m_inspectedEntity->getComponent<CAnimation>().layer, -1, 10);
		ImGui::Checkbox("Play Animation", &m_playAnimation);
	}
	if (ImGui::CollapsingHeader("Collision"))
	{
		ImGui::Checkbox("Box Collider", &boundingBox);
		if (boundingBox)
		{
			/*ImGui::Text("Angle: ");
			ImGui::SameLine();
			ImGui::SliderFloat("##Slider8", &angle, 0, 360);*/
		}
	}
	if (!m_playAnimation)
	{
		m_inspectedEntity->addComponent<CAnimation>(Animation(m_inspectedEntity->getComponent<CAnimation>().animation.getName(), m_assets[m_inspectedEntity->getComponent<CAnimation>().animation.getName()], frameCount, animSpeed), m_inspectedEntity->getComponent<CAnimation>().repeat);
	}
	m_inspectedEntity->addComponent<CTransform>(gridToMidPixel(gridX, gridY, m_inspectedEntity), scale, angle);
	if (boundingBox)
	{
		m_inspectedEntity->addComponent<CBoundingBox>(m_inspectedEntity->getComponent<CAnimation>().animation.getSize(), m_inspectedEntity->getComponent<CTransform>().angle);
	}
	else
	{
		m_inspectedEntity->removeComponent<CBoundingBox>();
	}
}
void LevelEditor::sGUI()
{
	ImGui::SetNextWindowPos(ImVec2(1265.f, 0.f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(315.f, 762.f), ImGuiCond_Always);
	ImGui::Begin("Level Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginChild("child1", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2)))
	{
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
				ImGui::Columns(ImGui::GetContentRegionAvailWidth() / 64.0f, nullptr, false);
				for (const auto& [name, texture] : m_assets)
				{
					float aspectRatio = (float)(texture.getSize().y) / (float)(texture.getSize().x);
					float height = 64.0f * aspectRatio;
					ImGui::BeginGroup();
					if (ImGui::ImageButton(texture, sf::Vector2f(64.0f, height)))
					{
						m_enableDragging = true;
						spawnEntity(name, texture);
					}
					ImGui::Text("%s", name.c_str());
					ImGui::EndGroup();
					ImGui::NextColumn();
				}
				ImGui::Columns(1);
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Entity Manager"))
			{
				for (auto& e : m_entityManager.getEntities())
				{
					ImGui::Text(std::to_string(e->id()).c_str());
					ImGui::SameLine();
					ImGui::Text((e->tag()).c_str());
					ImGui::SameLine();
					ImGui::Text(("(" + std::to_string((int)e->getComponent<CTransform>().pos.x) + ", " + std::to_string((int)e->getComponent<CTransform>().pos.y) + ")").c_str());
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
	}
	if (m_inspectedEntity && !m_enableDragging)
	{
		ImGui::Separator();
		if (ImGui::BeginChild("child2", ImGui::GetContentRegionAvail()))
		{
			if (ImGui::BeginTabBar("tab bar2"))
			{
				if (ImGui::BeginTabItem("Entity Inspector"))
				{
					entityInspectorGUI();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndChild();
		}
	}
	ImGui::End();
}

void LevelEditor::sRender()
{
	sf::RenderWindow& window = m_game->window();
	window.setView(m_gameView);
	window.clear();
	window.draw(m_gameBG);

	for (auto& e : m_entityManager.getEntities())
	{
		if (e->hasComponent<CAnimation>())
		{
			e->getComponent<CAnimation>().animation.getSprite().setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
			e->getComponent<CAnimation>().animation.getSprite().setScale(e->getComponent<CTransform>().scale.x, e->getComponent<CTransform>().scale.y);
			e->getComponent<CAnimation>().animation.getSprite().setRotation(e->getComponent<CTransform>().angle);
			window.draw(e->getComponent<CAnimation>().animation.getSprite());
		}
		if (m_drawCollision)
		{
			if (e->hasComponent<CBoundingBox>())
			{
				Vec2 rectSize = e->getComponent<CBoundingBox>().size;
				m_collisionRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
				m_collisionRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
				m_collisionRect.setPosition(e->getComponent<CTransform>().pos.x, e->getComponent<CTransform>().pos.y);
				window.draw(m_collisionRect);
			}
		}
	}
	if (m_inspectedEntity)
	{
		if (m_inspectedEntity->hasComponent<CBoundingBox>())
		{
			Vec2 rectSize = m_inspectedEntity->getComponent<CBoundingBox>().size;
			m_collisionRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
			m_collisionRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
			m_collisionRect.setPosition(m_inspectedEntity->getComponent<CTransform>().pos.x, m_inspectedEntity->getComponent<CTransform>().pos.y);
			m_collisionRect.setRotation(m_inspectedEntity->getComponent<CBoundingBox>().angle);
			window.draw(m_collisionRect);
		}
	}
	if (m_drawGrid)
	{
		for (int x = -50; x < 50; x++)
		{
			for (int y = -20; y < 20; y++)
			{
				Vec2 gridCellPos = gridToMidPixel(x, y);
				m_gridRect.setPosition(gridCellPos.x, gridCellPos.y);
				window.draw(m_gridRect);
				/*m_gridText.setString("(" + std::to_string(x) + "," + std::to_string(y) + ")");
				m_gridText.setPosition(gridCellPos.x - (m_gridSize.x / 2) + 5, gridCellPos.y - (m_gridSize.y / 2) + 5);
				window.draw(m_gridText);*/
			}
		}
	}

	Vec2 worldPos = windowToWorld(m_mousePos);
	//m_cursorDot.setPosition(worldPos.x, worldPos.y);
	m_cursorDot.setPosition(window.mapPixelToCoords(sf::Vector2i(m_mousePos.x, m_mousePos.y)));
	window.draw(m_cursorDot);
	window.setView(window.getDefaultView());

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
		else if (action.name() == "TOGGLE_COLLISION")
		{
			m_drawCollision = !m_drawCollision;
		}
		else if (action.name() == "DELETE")
		{
			if (m_inspectedEntity)
			{
				m_inspectedEntity->destroy();
				m_inspectedEntity = nullptr;
			}
		}
		else if (action.name() == "DUPLICATE")
		{
			if (m_inspectedEntity)
			{
				//m_inspectedEntity->getComponent<CDraggable>().dragging = false;
				snapToGrid(m_inspectedEntity);
				auto e = m_entityManager.addEntity(m_inspectedEntity);
				m_inspectedEntity = e;
			}
		}
		else if (action.name() == "LEFT_CLICK")
		{
			Vec2 wPos = windowToWorld(m_mousePos);
			// detect the picking up of entities
			for (auto e : m_entityManager.getEntities())
			{
				if (IsInside(wPos, e))
				{
					m_inspectedEntity = e;
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
		else if (action.name() == "RIGHT_CLICK")
		{
			m_enableDragging = !m_enableDragging;
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