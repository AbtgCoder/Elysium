#include "LevelEditor.h"

#include "Core/GameEngine.h"
#include "Physics/graham_scan.h"

#include "Asset/AssetManager.h"
#include "Asset/LevelImporter.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"


#include <cmath>
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
	registerAction(sf::Keyboard::T, "PLAY_LEVEL");
	registerAction(sf::Keyboard::LAlt, "ALT");
	registerAction(sf::Keyboard::W, "TRANSLATE_GIZMO");
	registerAction(sf::Keyboard::R, "SCALE_GIZMO");


	// Set ImGui Styles
	setImGuiStyle();

	// Debug drawing stuff
	m_gridRect.setSize(sf::Vector2f(m_gridSize.x, m_gridSize.y));
	m_gridRect.setOrigin(m_gridSize.x / 2, m_gridSize.y / 2);
	m_gridRect.setFillColor(sf::Color::Transparent);
	m_gridRect.setOutlineColor(sf::Color::White);
	m_gridRect.setOutlineThickness(1);
	m_collisionRect.setFillColor(sf::Color::Transparent);
	m_collisionRect.setOutlineColor(sf::Color::White);
	m_collisionRect.setOutlineThickness(1);

	m_EditorProjectPath = "D:/Game Development/Game_Engine_Programming/Elysium/Sandbox Project/Sandbox.eproject";
	OpenProject(m_EditorProjectPath);
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

Vec2 LevelEditor::windowToViewport(const Vec2& windowPos) const
{
	auto viewportPos = windowPos - m_viewportBounds.first;
	return viewportPos;
}
 

void LevelEditor::update()
{
	ImGui::SFML::Update(m_game->window(), m_game->m_deltaClock.restart());
	sRender();
	sGUI();
}

void LevelEditor::sRender()
{
	m_rt.create(m_viewportSize.x, m_viewportSize.y);
	m_levelView.setSize(m_viewportSize.x, m_viewportSize.y);
	m_levelView.zoom(m_levelViewZoom);
	m_rt.setView(m_levelView);

	if (m_ActiveLevel)
		m_ActiveLevel->OnUpdateEditor(m_rt);

}

void LevelEditor::NewProject()
{
}

bool LevelEditor::OpenProject()
{
	return false;
}

void LevelEditor::OpenProject(const std::filesystem::path& path)
{
	if (Project::Load(path))
	{
		AssetHandle startLevel = Project::GetActive()->GetConfig().StartLevel;
		if (startLevel)
			OpenLevel(startLevel);
		m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>(Project::GetActive());
	}
}

void LevelEditor::SaveProject()
{
}

void LevelEditor::NewLevel()
{
}

void LevelEditor::OpenLevel()
{
}

void LevelEditor::OpenLevel(AssetHandle handle)
{
	// assert handle

	m_ActiveLevel = AssetManager::GetAsset<Level>(handle);
	if (!m_ActiveLevel)
	{
		return;
	}
	m_LevelHierarchyPanel.SetLevel(m_ActiveLevel);
	m_EditorLevelPath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle);
}

void LevelEditor::SaveLevel()
{
	if (!m_EditorLevelPath.empty())
	{
		SerializeLevel(m_ActiveLevel, m_EditorLevelPath);
	}
}

void LevelEditor::SerializeLevel(std::shared_ptr<Level> level, const std::filesystem::path& path)
{
	LevelImporter::SaveLevel(level, path);
}

std::vector<Vec2> LevelEditor::generatePolygonColliderVertices(Entity entity)
{
	sf::Texture tex; // = m_assets[entity->getComponent<CAnimation>().animation.getName()];
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
				boundaryPoints.push_back(Vec2((float)x, (float)imageSize.y - y));
			}
		}
	}

	// TODO: reducing points ?? ramer-douglas-peucker algorithm

	// TODO: more algs :  jarvis march, chan's algorithm etc
	std::vector<Vec2> convexHull = grahamScan(boundaryPoints);

	return convexHull;
}


void LevelEditor::sGUI()
{
	static bool dockspaceOpen = true;
	static bool opt_fullsreen = true;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullsreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
	{
		window_flags |= ImGuiWindowFlags_NoBackground;
	}

	ImGui::Begin("Dockspace demo", &dockspaceOpen, window_flags);

	if (opt_fullsreen)
	{
		ImGui::PopStyleVar(2);
	}

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Level", "Ctrl+O"))
			{
				std::cout << "wow load level!\n";
			}
			ImGui::Separator();
			if (ImGui::MenuItem("New Level", "Ctrl+N"))
			{
				std::cout << "wow new level!\n";
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	m_LevelHierarchyPanel.OnImGuiRender();
	m_ContentBrowserPanel->OnImGuiRender();

	ImGui::Begin("Viewport");
	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_viewportBounds.first = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_viewportBounds.second = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
	ImGui::Image(m_rt);

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
		{
			AssetHandle handle = *(AssetHandle*)payload->Data;
			if (AssetManager::GetAssetType(handle) == AssetType::Level)
			{
				OpenLevel(handle);
			}
			else if (AssetManager::GetAssetType(handle) == AssetType::Texture)
			{
				// spawn entity with sprite renderer component
				Vec2 viewportPos = windowToViewport(m_mousePos);
				Vec2 worldPos = m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));
				Entity newEntity = m_ActiveLevel->AddEntityWithSprite(worldPos, handle);
				m_LevelHierarchyPanel.SetInspectedEntity(newEntity);
			}
			else
			{
				// log warning: wrong asset type!
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Gizmos
	m_inspectedEntity = m_LevelHierarchyPanel.GetInspectedEntity();
	if (m_inspectedEntity)
	{
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		static const ImU32 directionColor[3] = { 0xFF715ED8, 0xFF25AA25, 0xFFCC532C }; // x, y, z direction colors
		static const ImU32 selectionColor = 0xFF20AACC;
		Vec2 ePos = m_inspectedEntity.getComponent<CTransform>().pos;
		sf::Vector2i pixel = m_rt.mapCoordsToPixel(sf::Vector2f(ePos.x, ePos.y));
		ImVec2 origin = ImVec2(m_viewportBounds.first.x + pixel.x, m_viewportBounds.first.y + pixel.y);
		static const float lineLength = 80.0f;
		static const float lineThickness = 4.0f;
		static const float arrowSize = 6.0f;
		static const float rectSize = 6.0f;

		// X-translation gizmo
		ImU32 colorX = (m_gizmoSelectX || m_gizmoHoverX) ? selectionColor : directionColor[0];
		// line
		ImVec2 endPointX = ImVec2(origin.x + lineLength, origin.y);
		drawList->AddLine(origin, endPointX, colorX, lineThickness);
		// Y-translation gizmo
		ImU32 colorY = (m_gizmoSelectY || m_gizmoHoverY) ? selectionColor : directionColor[1];
		// line
		ImVec2 endPointY = ImVec2(origin.x, origin.y - lineLength);
		drawList->AddLine(origin, endPointY, colorY, lineThickness);
		if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
		{
			// X-arrow
			ImVec2 dir = ImVec2(origin.x - endPointX.x, origin.y - endPointX.y);
			float d = sqrtf(ImLengthSqr(dir));
			dir.x = dir.x / d * arrowSize;
			dir.y = dir.y / d * arrowSize;
			ImVec2 orthogonoalDir(dir.y * 0.8f, -dir.x * 0.8f);
			ImVec2 a = ImVec2(endPointX.x + dir.x, endPointX.y + dir.y);
			drawList->AddTriangleFilled(ImVec2(endPointX.x - dir.x, endPointX.y - dir.y), ImVec2(a.x + orthogonoalDir.x, a.y + orthogonoalDir.y), ImVec2(a.x - orthogonoalDir.x, a.y - orthogonoalDir.y), colorX);
			// Y-arrow
			dir = ImVec2(origin.x - endPointY.x, origin.y - endPointY.y);
			d = sqrtf(ImLengthSqr(dir));
			dir.x = dir.x / d * arrowSize;
			dir.y = dir.y / d * arrowSize;
			orthogonoalDir = ImVec2(dir.y * 0.8f, -dir.x * 0.8f);
			a = ImVec2(endPointY.x + dir.x, endPointY.y + dir.y);
			drawList->AddTriangleFilled(ImVec2(endPointY.x - dir.x, endPointY.y - dir.y), ImVec2(a.x + orthogonoalDir.x, a.y + orthogonoalDir.y), ImVec2(a.x - orthogonoalDir.x, a.y - orthogonoalDir.y), colorY);
		}
		else if (m_gizmoType == GIZMO_OPERATION::SCALE)
		{
			// X-square
			drawList->AddRectFilled(ImVec2(endPointX.x, endPointX.y - rectSize), ImVec2(endPointX.x + 2*rectSize, endPointX.y + rectSize), colorX);
			// Y-square	
			drawList->AddRectFilled(ImVec2(endPointY.x - rectSize, endPointY.y - 2*rectSize), ImVec2(endPointY.x + rectSize, endPointY.y), colorY);
		}
		
		if (ImGui::IsMouseHoveringRect(ImVec2(origin.x, origin.y - arrowSize), ImVec2(endPointX.x + arrowSize, endPointX.y + arrowSize)))
		{
			m_gizmoHoverX = true;
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m_gizmoSelectX = true;
				m_lastGizmoPosX = windowToViewport(m_mousePos);
			}
			if (m_gizmoSelectX && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				m_gizmoSelectX = false;
			}
		}
		else
		{
			m_gizmoHoverX = false;
		}

		if (ImGui::IsMouseHoveringRect(ImVec2(endPointY.x - arrowSize, endPointY.y - arrowSize), ImVec2(origin.x + arrowSize, origin.y)))
		{
			m_gizmoHoverY = true;
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m_gizmoSelectY = true;
				m_lastGizmoPosY = windowToViewport(m_mousePos);
			}
			if (m_gizmoSelectY && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
			{
				m_gizmoSelectY = false;
			}
		}
		else
		{
			m_gizmoHoverY = false;
		}

	}

	ImGui::End(); // end "Viewport"

	ImGui::End(); // end "Dockspace demo" 

	ImGui::SFML::Render(m_game->window());

}


void LevelEditor::sDoAction(const Action& action)
{
	if (action.name() == "MOUSE_MOVE")
	{
		m_mousePos = action.pos();
		Vec2 viewportPos = windowToViewport(m_mousePos);
		Vec2 deltaPos = m_lastLevelViewPos - m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));

		if (m_levelViewMoving)
		{
			m_levelView.setCenter(m_levelView.getCenter() + sf::Vector2f(deltaPos.x, deltaPos.y));
		}

		// Gizmo System
		if (m_gizmoSelectX && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoPosX;
			m_lastGizmoPosX = viewportPos;
			if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
			{
				m_inspectedEntity.getComponent<CTransform>().pos.x += deltaPos.x;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().scale.x += m_scalingFactor * deltaPos.x;
			}
		}
		else if (m_gizmoSelectY && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoPosY;
			m_lastGizmoPosY = viewportPos;
			if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
			{
				m_inspectedEntity.getComponent<CTransform>().pos.y += deltaPos.y;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().scale.y += m_scalingFactor * deltaPos.y;
			}
		}
	}

	if (action.name() == "MOUSE_WHEEL_SCROLL")
	{
		if (!m_levelViewMoving && m_altPressed)
		{
			float delta = action.pos().x;
			if (delta <= -1)
			{
				m_levelViewZoom = std::min(2.0f, m_levelViewZoom + 0.1f);
			}
			else if (delta >= 1)
			{
				m_levelViewZoom = std::max(0.5f, m_levelViewZoom - 0.1f);
			}
		}
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
		else if (action.name() == "ALT")
		{
			m_altPressed = true;
		}
		else if (action.name() == "TRANSLATE_GIZMO")
		{
			m_gizmoType = GIZMO_OPERATION::TRANSLATE;
		}
		else if (action.name() == "SCALE_GIZMO")
		{
			m_gizmoType = GIZMO_OPERATION::SCALE;
		}
		else if (action.name() == "DELETE")
		{
			Entity inspectedEntity = m_LevelHierarchyPanel.GetInspectedEntity();
			if (inspectedEntity)
			{
				m_LevelHierarchyPanel.SetInspectedEntity({});
				m_ActiveLevel->DestroyEntity(inspectedEntity);
			}
		}
		else if (action.name() == "DUPLICATE")
		{
			Entity inspectedEntity = m_LevelHierarchyPanel.GetInspectedEntity();
			if (inspectedEntity)
			{
				Entity newEntity = m_ActiveLevel->AddEntity(inspectedEntity);
				m_LevelHierarchyPanel.SetInspectedEntity(newEntity);
			}
		}
		else if (action.name() == "LEFT_CLICK")
		{
			Vec2 viewportPos = windowToViewport(m_mousePos);
			Vec2 worldPos = m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));

			if (viewportPos.x > 0 && viewportPos.x < m_viewportSize.x && viewportPos.y > 0 && viewportPos.y < m_viewportSize.y)
			{
				Entity entity = m_ActiveLevel->GetEntityIfClicked(worldPos);
				if (entity)
				{
					m_LevelHierarchyPanel.SetInspectedEntity(entity);
				}
				else
				{
					if (!(m_gizmoHoverX || m_gizmoHoverY || m_gizmoSelectX || m_gizmoSelectY))
					{
						m_LevelHierarchyPanel.SetInspectedEntity({});

					}
				}
			}

			if (m_altPressed)
			{
				m_levelViewMoving = true;
				m_lastLevelViewPos = worldPos;
			}

		}
		else if (action.name() == "QUIT")
		{
			SaveLevel();
			m_hasEnded = true;
			onEnd();
		}
		else if (action.name() == "PLAY_LEVEL")
		{
			// Level state, level play
		}
	}
	
	if (action.type() == "END")
	{
		if (action.name() == "ALT")
		{
			m_altPressed = false;
			m_levelViewMoving = false;
		}
		if (action.name() == "LEFT_CLICK")
		{
			if (m_altPressed)
			{
				m_levelViewMoving = false;
			}
			if (m_gizmoSelectX)
			{
				m_gizmoSelectX = false;
			}
			if (m_gizmoSelectY)
			{
				m_gizmoSelectY = false;
			}
		}
	}
}

void LevelEditor::onEnd()
{
	m_game->quit();
}