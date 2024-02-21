#include "GameEngine.h"
#include "LevelEditor.h"
#include "Physics.h"
#include "graham_scan.h"
#include "Scene_Play.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"

#include <yaml-cpp/yaml.h>

#include <filesystem>
#include <cmath>
#include <fstream>
#include <algorithm>

LevelEditor::LevelEditor(GameEngine* gameEngine)
	: Scene(gameEngine)
{
	init();
}

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

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

void LevelEditor::init()
{
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	registerAction(sf::Keyboard::C, "TOGGLE_COLLISION");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Delete, "DELETE");
	registerAction(sf::Keyboard::D, "DUPLICATE");
	registerAction(sf::Keyboard::T, "PLAY_LEVEL");

	// Set ImGui Styles
	setImGuiStyle();
	m_BaseDirectory.assign("../../../Assets/");
	m_CurrentDirectory.assign("../../../Assets/");
	m_DirectoryIcon = m_game->assets().getTexture("DirectoryIcon");
	m_FileIcon = m_game->assets().getTexture("FileIcon");

	m_gameView.reset(sf::FloatRect(0, 0, 1262, 762));
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
	loadLevel("../../../Assets/levels/level_test.elysium");
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

bool LevelEditor::loadLevel(const std::filesystem::path& filepath)
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
			std::shared_ptr<Entity> deserializedEntity = m_entityManager.addEntity(tag);
			deserializedEntity->addComponent<CTag>(tag);

			auto transformComponent = entity["Transform"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity->addComponent<CTransform>();
				tc.pos = transformComponent["Position"].as<Vec2>();
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

			deserializedEntity->addComponent<CDraggable>();
		}
	}

	return true;
}

static void serializeEntity(YAML::Emitter& out, std::shared_ptr<Entity> entity)
{
	out << YAML::BeginMap; // Entity
	out << YAML::Key << "Entity" << YAML::Value << entity->getComponent<CTag>().tag;
	if (entity->hasComponent<CTransform>())
	{
		out << YAML::Key << "Transform";
		out << YAML::BeginMap; // Transform Component

		auto& tc = entity->getComponent<CTransform>();
		out << YAML::Key << "Position" << YAML::Value << tc.pos;
		out << YAML::Key << "Scale" << YAML::Value << tc.scale;
		out << YAML::Key << "Angle" << YAML::Value << tc.angle;

		out << YAML::EndMap; // TransformComponent
	}
	if (entity->hasComponent<CAnimation>())
	{
		out << YAML::Key << "Animation";
		out << YAML::BeginMap;

		auto& ac = entity->getComponent<CAnimation>();
		out << YAML::Key << "Texture" << YAML::Value << ac.animation.getName();
		out << YAML::Key << "Speed" << YAML::Value << ac.animSpeed;
		out << YAML::Key << "Frames" << YAML::Value << ac.frameCount;
		out << YAML::Key << "Repeatable" << YAML::Value << ac.repeat;
		out << YAML::Key << "Layer" << YAML::Value << ac.layer;

		out << YAML::EndMap;
	}
	if (entity->hasComponent<CBoundingBox>())
	{
		out << YAML::Key << "AABB";
		out << YAML::BeginMap;

		auto& bc2d = entity->getComponent<CBoundingBox>();
		out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
		out << YAML::Key << "Size" << YAML::Value << bc2d.size;

		out << YAML::EndMap;
	}
	if (entity->hasComponent<CPolygonCollider>())
	{
		out << YAML::Key << "PolygonCollider";
		out << YAML::BeginMap;

		auto& pc2d = entity->getComponent<CPolygonCollider>();
		out << YAML::Key << "Offset" << YAML::Value << pc2d.offset;
		out << YAML::Key << "Points" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Size" << YAML::Value << pc2d.colliderVertices.size();
		out << YAML::Key << "Elements" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		for (auto& point : pc2d.colliderVertices)
		{
			out << point;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		out << YAML::EndMap;
	}
	if (entity->hasComponent<CGravity>())
	{
		out << YAML::Key << "Gravity";
		out << YAML::BeginMap;

		auto& gc = entity->getComponent<CGravity>();
		out << YAML::Key << "Gravity" << YAML::Value << gc.gravity;
		out << YAML::EndMap;
	}
	out << YAML::EndMap;
}

void LevelEditor::saveLevel()
{
	std::filesystem::path filepath = "../../../Assets/levels/level_test.elysium";

	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Level" << YAML::Value << "Untitled";
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	for (auto entity : m_entityManager.getEntities())
	{
		if (entity->isActive())
		{
			serializeEntity(out, entity);
		}
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();

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
	e->addComponent<CTag>("Tile");
	e->addComponent<CAnimation>(Animation(name, tex), true, 0);
	e->addComponent<CTransform>(m_mousePos);
	e->addComponent<CDraggable>(true);
	m_inspectedEntity = e;
}

void LevelEditor::update()
{
	ImGui::SFML::Update(m_game->window(), m_game->m_deltaClock.restart());
	m_entityManager.update();
	if (m_enableDragging)
	{
		sDrag();
	}
	sRender();
	sGUI();
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

std::vector<Vec2> LevelEditor::generatePolygonColliderVertices(std::shared_ptr<Entity> entity)
{
	sf::Texture tex = m_assets[entity->getComponent<CAnimation>().animation.getName()];
	sf::Image image = tex.copyToImage();
	sf::Vector2u imageSize = image.getSize();

	sf::Image paddedBinaryImage;
	paddedBinaryImage.create(imageSize.x + 2, imageSize.y + 2);

	for (int y = 0; y < imageSize.y + 2; ++y) {
		for (int x = 0; x < imageSize.x + 2; ++x) {
			if (y - 1 >= 0 && y - 1 < imageSize.y && x - 1 >= 0 && x - 1 < imageSize.x)
			{
				sf::Color pixelColor = image.getPixel(x - 1, y - 1);
				int grayscaleColor = static_cast<int>((pixelColor.r + pixelColor.g + pixelColor.b) / 3);
				if (grayscaleColor != 0)
				{
					paddedBinaryImage.setPixel(x, y, sf::Color::White);
				}
				else
				{
					paddedBinaryImage.setPixel(x, y, sf::Color::Black);
				}
			}
			else
			{
				paddedBinaryImage.setPixel(x, y, sf::Color::Black);
			}
		}
	}


	// boundaryPoints = countourTracing(paddedBinaryImage) TODO: moore neighborhood contour tracing ??
	std::vector<Vec2> boundaryPoints;
	for (uint32_t y = 0; y < imageSize.y; ++y)
	{
		for (uint32_t x = 0; x < imageSize.x; ++x)
		{
			if (paddedBinaryImage.getPixel(x, y) == sf::Color::White &&
				(paddedBinaryImage.getPixel(x - 1, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y - 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x - 1, y) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x - 1, y + 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x, y + 1) == sf::Color::Black ||
					paddedBinaryImage.getPixel(x + 1, y + 1) == sf::Color::Black))
			{
				boundaryPoints.push_back(Vec2(x, imageSize.y - y));
			}
		}
	}

	// TODO: reducing points ?? ramer-douglas-peucker algorithm

	// TODO: more algs :  jarvis march, chan's algorithm etc
	std::vector<Vec2> convexHull = grahamScan(boundaryPoints);

	return convexHull;
}

template<typename T, typename UIFunction>
static void DrawComponentGUI(const std::string& name, std::shared_ptr<Entity> entity, UIFunction uiFunction)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity->hasComponent<T>())
	{
		auto& component = entity->getComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvailable.x - lineHeight);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				removeComponent = true;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			entity->removeComponent<T>();
		}
	}
}

static void DrawVec2Control(const std::string& label, Vec2& values, float resetValue = 0.0f, float columnWidth = 64.0f)
{
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 3.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// TODO: button styles ??
	if (ImGui::Button("X", buttonSize))
	{
		values.x = resetValue;
	}

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("Y", buttonSize))
	{
		values.y = resetValue;
	}

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

static void DrawFloatControl(const std::string& label, float& value, float vMin = 0.0f, float vMax = 360.0f, float columnWidth = 80.0f)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImVec2 contentPos = ImGui::GetCursorPos();
	float columnWidth2 = ImGui::GetColumnWidth();
	float widgetWidth = ImGui::CalcItemWidth();
	contentPos.x += (columnWidth2 - widgetWidth) / 2;
	ImGui::SetCursorPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });
	ImGui::DragFloat("##val", &value, 0.1f, vMin, vMax, "%.2f");
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

static void DrawIntControl(const std::string& label, int& value, int vMin = 0, int vMax = 120, float columnWidth = 80.0f)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImVec2 contentPos = ImGui::GetCursorPos();
	float columnWidth2 = ImGui::GetColumnWidth();
	float widgetWidth = ImGui::CalcItemWidth();
	contentPos.x += (columnWidth2 - widgetWidth) / 2;
	ImGui::SetCursorPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });
	ImGui::DragInt("##val", &value, 0.1f, vMin, vMax);
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

void LevelEditor::entityInspectorGUI()
{
	auto& tag = m_inspectedEntity->getComponent<CTag>().tag;
	char buffer[256];
	memset(buffer, 0, sizeof(buffer));
	strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
	if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
	{
		tag = std::string(buffer);
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponent");
	}

	if (ImGui::BeginPopup("AddComponent"))
	{
		DisplayAddComponentEntry<CTransform>("Transform");
		DisplayAddComponentEntry<CAnimation>("Animation");
		DisplayAddComponentEntry<CGravity>("Gravity");
		if (m_inspectedEntity->hasComponent<CAnimation>())
		{
			DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D", m_inspectedEntity->getComponent<CAnimation>().animation.getSize());
			DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D", generatePolygonColliderVertices(m_inspectedEntity));
		}
		else
		{
			DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D");
			DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D");
		}
		ImGui::EndPopup();
	}

	ImGui::PopItemWidth();


	DrawComponentGUI<CTransform>("Transform", m_inspectedEntity, [](auto& component)
		{
			DrawVec2Control("Position", component.pos, 0.0f, 80.0f);
			DrawVec2Control("Scale", component.scale, 0.0f, 80.0f);
			DrawFloatControl("Angle", component.angle, 0.0f, 360.0f);
		});

	DrawComponentGUI<CAnimation>("Animation", m_inspectedEntity, [](auto& component)
		{
			float imgSize = 80.0f;
			ImGui::PushID("Sprite");
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 80.0f);
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imgSize / 2 - 7.0f);
			ImGui::Text("Sprite");
			ImGui::NextColumn();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - imgSize) / 2);
			float aspectRatio = (float)(component.animation.getSize().y) / (float)(component.animation.getSize().x);
			float imgHeight = imgSize * aspectRatio;
			float diff = imgSize - imgHeight;
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);
			ImGui::Image(component.animation.getSprite(), sf::Vector2f(imgSize, imgHeight));
			ImGui::Columns(1);
			ImGui::PopID();

			int animSpeed = (int)component.animSpeed;
			DrawIntControl("Speed", animSpeed, 0, 120);
			component.animSpeed = animSpeed;

			int frameCount = (int)component.frameCount;
			DrawIntControl("Frames", frameCount, 1, 20);
			component.frameCount = frameCount;

			DrawIntControl("Layer", component.layer, -1, 10);

			ImGui::Checkbox("Repeatable", &component.repeat);
			//ImGui::Checkbox("Play Animation", &m_playAnimation);
		});

	DrawComponentGUI<CBoundingBox>("Box Collider 2D", m_inspectedEntity, [](auto& component)
		{
			DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
			DrawVec2Control("Size", component.size, 0.0f, 80.0f);
			component.halfSize = component.size / 2;
		});

	DrawComponentGUI<CPolygonCollider>("Polygon Collider 2D", m_inspectedEntity, [](auto& component)
		{
			DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
			if (ImGui::CollapsingHeader("Points"))
			{
				for (size_t i = 0; i < component.colliderVertices.size(); i++)
				{
					std::string label = "Point " + std::to_string(i);
					DrawVec2Control(label, component.colliderVertices[i], 0.0f, 80.0f);
				}
			}
		});

	DrawComponentGUI<CGravity>("Gravity", m_inspectedEntity, [](auto& component)
		{
			DrawFloatControl("Gravity", component.gravity, 0.0f, 10.0f);
		});
}

template<typename T, typename... TArgs>
void LevelEditor::DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs)
{
	if (!m_inspectedEntity->hasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			m_inspectedEntity->addComponent<T>(std::forward<TArgs>(mArgs)...);
			ImGui::CloseCurrentPopup();
		}
	}
}

void LevelEditor::contentBrowserGUI()
{
	if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
	{
		if (ImGui::Button("<-"))
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
	}

	static float padding = 8.0f;
	static float thumbnailSize = 64.0f;
	float cellSize = thumbnailSize + padding;

	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnCount = (int)(panelWidth / cellSize);
	if (columnCount < 1)
	{
		columnCount = 1;
	}

	ImGui::Columns(columnCount, 0, false);

	for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
	{
		const auto& path = directoryEntry.path();
		std::string filenameString = path.filename().string();
		ImGui::PushID(filenameString.c_str());
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (directoryEntry.is_directory())
		{
			ImGui::ImageButton(m_DirectoryIcon, { thumbnailSize, thumbnailSize });
		}
		else
		{
			auto it = m_assets.find(directoryEntry.path().stem().string());
			if (it != m_assets.end())
			{
				sf::Texture& texture = it->second;
				float aspectRatio = (float)(texture.getSize().y) / (float)(texture.getSize().x);
				float thumbnailHeight = thumbnailSize * aspectRatio;
				float diff = thumbnailSize - thumbnailHeight;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);
				if (ImGui::ImageButton(texture, { thumbnailSize, thumbnailHeight }))
				{
					spawnEntity(directoryEntry.path().stem().string(), texture);
				}
			}
			else
			{
				ImGui::ImageButton(m_FileIcon, { thumbnailSize, thumbnailSize });
			}

			if (directoryEntry.path().extension().string() == ".elysium")
			{
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Load Level"))
					{
						loadLevel(directoryEntry.path());
					}
					ImGui::EndPopup();
				}
			}
		}
		
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (directoryEntry.is_directory())
			{
				m_CurrentDirectory /= path.filename();
			}
		}
		ImGui::TextWrapped(filenameString.c_str());

		ImGui::NextColumn();
		ImGui::PopID();
	}
	ImGui::Columns(1);

	/*ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
	ImGui::SliderFloat("Padding", &padding, 0, 32);*/
}

void LevelEditor::entityManagerGUI()
{
	for (auto& e : m_entityManager.getEntities())
	{
		drawEntityNode(e);
	}

	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
	{
		m_inspectedEntity = nullptr;
	}

	// Right click on blank space
	if (ImGui::BeginPopupContextWindow(0, 1, false))
	{
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			auto entity = m_entityManager.addEntity("Empty Entity");
			entity->addComponent<CTag>("Empty Entity");
		}
		ImGui::EndPopup();
	}
}

void LevelEditor::drawEntityNode(std::shared_ptr<Entity> entity)
{
	auto& tag = entity->getComponent<CTag>().tag;

	ImGuiTreeNodeFlags flags = ((m_inspectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity->id()), flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_inspectedEntity = entity;
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
		{
			entityDeleted = true;
		}
		ImGui::EndPopup();
	}

	if (opened)
	{
		// TODO: add entity info  ?? (id, pos, etc)
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
		if (opened)
		{
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		entity->destroy();
		if (m_inspectedEntity == entity)
		{
			m_inspectedEntity = nullptr;
		}
	}
}

void LevelEditor::sGUI()
{
	ImVec2 guiWinSize;
	guiWinSize.x = 315.0f;
	guiWinSize.y = m_game->window().getSize().y;
	ImGui::SetNextWindowPos(ImVec2(1265.0f, 0.0f));
	ImGui::SetNextWindowSize(guiWinSize);


	ImGui::Begin("Level Editor", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginChild("child1", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y / 2)))
	{
		if (ImGui::BeginTabBar("tab bar"))
		{
#if 0
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
#endif

			if (ImGui::BeginTabItem("Assets"))
			{
				contentBrowserGUI();
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Entity Manager"))
			{
				entityManagerGUI();
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
	ImGui::SFML::Render(m_game->window());
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
		if (m_drawCollision || e == m_inspectedEntity)
		{
			if (e->hasComponent<CBoundingBox>())
			{
				Vec2 rectSize = e->getComponent<CBoundingBox>().size;
				Vec2 offset = e->getComponent<CBoundingBox>().offset;
				m_collisionRect.setSize(sf::Vector2f(rectSize.x, rectSize.y));
				m_collisionRect.setOrigin(rectSize.x / 2, rectSize.y / 2);
				m_collisionRect.setPosition(e->getComponent<CTransform>().pos.x + offset.x, e->getComponent<CTransform>().pos.y + offset.y);
				window.draw(m_collisionRect);
			}
			if (e->hasComponent<CPolygonCollider>())
			{
				sf::ConvexShape polygon;
				std::vector<Vec2> vertices = e->getComponent<CPolygonCollider>().colliderVertices;
				Vec2 offset = e->getComponent<CPolygonCollider>().offset;
				Vec2 ePos = e->getComponent<CTransform>().pos;
				Vec2 eSize = e->getComponent<CAnimation>().animation.getSize();
				polygon.setPointCount(vertices.size());
				for (size_t i = 0; i < vertices.size(); i++)
				{
					polygon.setPoint(i, sf::Vector2f(ePos.x + offset.x - eSize.x/2 + vertices[i].x, ePos.y + offset.y + eSize.y/2 - vertices[i].y));
				}
				polygon.setFillColor(sf::Color::Transparent);
				polygon.setOutlineColor(sf::Color::White);
				polygon.setOutlineThickness(1);
				window.draw(polygon);
			}
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
			saveLevel();
			m_hasEnded = true;
			onEnd();
		}
		else if (action.name() == "PLAY_LEVEL")
		{
			saveLevel();
			std::string levelPath = "../../../Assets/levels/level_test.elysium";
 			std::shared_ptr<Scene> scenePlay = std::make_shared<Scene_Play>(m_game, levelPath);
			m_game->changeScene("Play", scenePlay, true);
		}
	}
	
}


void LevelEditor::onEnd()
{
	m_game->quit();

}