#include "EditorLayer.h"

#include "Core/Application.h"
#include "Physics/graham_scan.h"

#include "Asset/AssetManager.h"
#include "Asset/SceneImporter.h"
#include "Asset/TextureImporter.h"

#include "Utils/FileUtils.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"


#include <cmath>
#include <algorithm>

EditorLayer::EditorLayer(Application* Application)
	: Layer(Application)
{
	init();
}

void EditorLayer::init()
{
	registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	registerAction(sf::Keyboard::Escape, "QUIT");
	registerAction(sf::Keyboard::Delete, "DELETE");
	registerAction(sf::Keyboard::D, "DUPLICATE");
	registerAction(sf::Keyboard::P, "PLAY_Scene");
	registerAction(sf::Keyboard::LAlt, "ALT");
	registerAction(sf::Keyboard::W, "TRANSLATE_GIZMO");
	registerAction(sf::Keyboard::R, "SCALE_GIZMO");
	registerAction(sf::Keyboard::E, "ROTATION_GIZMO");



	m_IconPlay = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PlayButton.png");
	m_IconPause = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PauseButton.png");
	m_IconStep = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StepButton.png");
	m_IconStop = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StopButton.png");



	// Set ImGui Styles
	setImGuiStyle();

	// Debug drawing stuff
	m_gridRect.setSize(sf::Vector2f(m_gridSize.x, m_gridSize.y));
	m_gridRect.setOrigin(m_gridSize.x / 2, m_gridSize.y / 2);
	m_gridRect.setFillColor(sf::Color::Transparent);
	m_gridRect.setOutlineColor(sf::Color::White);
	m_gridRect.setOutlineThickness(1);


	OpenProject();
}

void EditorLayer::setImGuiStyle()
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

Vec2 EditorLayer::windowToViewport(const Vec2& windowPos) const
{
	auto viewportPos = windowPos - m_viewportBounds.first;
	return viewportPos;
}
 

void EditorLayer::update(float ts)
{
	ImGui::SFML::Update(m_game->window(), m_game->m_deltaClock.restart());
	m_rt.create(m_viewportSize.x, m_viewportSize.y);
	m_SceneView.setSize(m_viewportSize.x, m_viewportSize.y);
	m_SceneView.zoom(m_SceneViewZoom);
	m_rt.setView(m_SceneView);
	
	switch (m_SceneState)
	{
	case SceneState::Edit:
	{
		if (m_ActiveScene)
			m_ActiveScene->OnUpdateEditor(m_rt);
		break;
	}
	case SceneState::Play:
	{
		if (m_ActiveScene)
			m_ActiveScene->OnUpdateRuntime(m_rt, ts);
		break;
	}
	}

	
	sRender();
	sGUI();
}

void EditorLayer::sRender()
{

}

void EditorLayer::NewProject()
{
}

bool EditorLayer::OpenProject()
{
	std::string projectPath = WindowsFileUtils::OpenFile(m_game->window().getSystemHandle(), "Elysium Project (*.eproject)\0*.eproject\0");
	if (projectPath.empty())
		return false;

	OpenProject(projectPath);
	return true;
}

void EditorLayer::OpenProject(const std::filesystem::path& path)
{
	if (Project::Load(path))
	{
		m_EditorProjectPath = path;
		AssetHandle lastOpenedScene = Project::GetActive()->GetConfig().lastOpenedScene;
		AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
		if (lastOpenedScene)
			OpenScene(lastOpenedScene);
		else if (startScene)
			OpenScene(startScene);
		m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>(Project::GetActive());
	}
}

void EditorLayer::SaveProject()
{
	Project::SaveActive(m_EditorProjectPath);
}

void EditorLayer::NewScene()
{
	std::string path = WindowsFileUtils::SaveFile(m_game->window().getSystemHandle(), "Elysium Scene (*.elysium)\0*.elysium\0");
	if (!path.empty())
	{
		auto relativePath = std::filesystem::relative(path, Project::GetActiveAssetDirectory());
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		SceneImporter::SaveScene(scene, relativePath);
		Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
		// refresh content browser
		m_ContentBrowserPanel->RefreshAssetTree();
		const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : assetRegistry)
		{
			if (metadata.FilePath == relativePath)
			{
				OpenScene(handle);
				break;
			}
		}
	}
}

void EditorLayer::OpenScene()
{
	std::string path = WindowsFileUtils::OpenFile(m_game->window().getSystemHandle(), "Elysium Scene (*.elysium)\0*.elysium\0");
	if (!path.empty())
	{
		auto relativePath = std::filesystem::relative(path, Project::GetActiveAssetDirectory());
		Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
		// refresh content browser
		m_ContentBrowserPanel->RefreshAssetTree();
		const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
		for (const auto& [handle, metadata] : assetRegistry)
		{
			if (metadata.FilePath == relativePath)
			{
				OpenScene(handle);
				break;
			}
		}
	}
}

void EditorLayer::OpenScene(AssetHandle handle)
{
	// assert handle

	m_ActiveScene = AssetManager::GetAsset<Scene>(handle);
	if (!m_ActiveScene)
	{
		return;
	}
	Project::SetLastOpenedScene(handle);
	m_SceneHierarchyPanel.SetScene(m_ActiveScene);
	m_PhysicsConfigPanel.SetScene(m_ActiveScene);
	m_EditorScene = m_ActiveScene;
	m_EditorScenePath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(handle);
}

void EditorLayer::SaveScene()
{
	if (!m_EditorScenePath.empty())
	{
		SerializeScene(m_ActiveScene, m_EditorScenePath);
	}
}

void EditorLayer::SerializeScene(std::shared_ptr<Scene> Scene, const std::filesystem::path& path)
{
	SceneImporter::SaveScene(Scene, path);
}

void EditorLayer::OnScenePlay()
{
	m_ActiveScene = Scene::Copy(m_EditorScene);
	m_ActiveScene->OnRuntimeStart();
	m_SceneState = SceneState::Play;
	m_SceneHierarchyPanel.SetScene(m_ActiveScene);
	m_PhysicsConfigPanel.SetScene(m_ActiveScene);
}

void EditorLayer::OnSceneStop()
{
	m_ActiveScene->OnRuntimeStop();
	m_SceneState = SceneState::Edit;
	m_ActiveScene = m_EditorScene;
	m_SceneHierarchyPanel.SetScene(m_ActiveScene);
	m_PhysicsConfigPanel.SetScene(m_ActiveScene);
}


void EditorLayer::sGUI()
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

	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
			{
				SaveScene(); // save active Scene
				OpenScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
			{
				std::cout << "wow new Scene!\n";
				NewScene();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	m_SceneHierarchyPanel.OnImGuiRender();
	m_ContentBrowserPanel->OnImGuiRender();
	m_PhysicsConfigPanel.OnImGuiRender();

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
			if (AssetManager::GetAssetType(handle) == AssetType::Scene)
			{
				if (m_SceneState == SceneState::Play)
				{
					OnSceneStop();
				}
				SaveScene();
				OpenScene(handle);
			}
			else if (AssetManager::GetAssetType(handle) == AssetType::Texture)
			{
				// spawn entity with sprite renderer component
				Vec2 viewportPos = windowToViewport(m_mousePos);
				Vec2 worldPos = m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));
				Entity newEntity = m_ActiveScene->AddEntityWithSprite(worldPos, handle);
				m_SceneHierarchyPanel.SetInspectedEntity(newEntity);
			}
			else
			{
				// log warning: wrong asset type!
			}
		}
		ImGui::EndDragDropTarget();
	}

	// Gizmos
	m_inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();
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
		static const float circleRadius = 80.0f;
		static const float squareSize = 20.0f;

		if (m_gizmoType == GIZMO_OPERATION::ROTATE)
		{
			// circle
			ImU32 color = (m_gizmoRotateSelect || m_gizmoRotateHover) ? selectionColor : directionColor[2];
			drawList->AddCircle(origin, circleRadius, color, 64);
			// convex poly filled
			ImVec2 circleArcPoints[32 + 1];
			circleArcPoints[0] = origin;
			float startAngle = fmod(m_inspectedEntity.getComponent<CTransform>().angle + 180.0f, 360.0f);
			if (startAngle < 0)
				startAngle += 360.0f;
			startAngle -= 180.0f;
			float endAngle = m_gizmoRotateSelect  ? - 1 * atan2(windowToViewport(m_mousePos).y - pixel.y, windowToViewport(m_mousePos).x - pixel.x) * 180.0 / 3.14 : startAngle;

			float angle_step = (endAngle - startAngle) / 32;
			for (unsigned int i = 1; i < 32; i++)
			{
				float angle = startAngle + angle_step * i;
				float costheta = cos(angle * 3.14 / 180.0);
				float sintheta = sin(angle * 3.14 / 180.0);
				circleArcPoints[i] = ImVec2(origin.x + circleRadius * costheta, origin.y - circleRadius * sintheta);
			}
			drawList->AddConvexPolyFilled(circleArcPoints, 32, 0x8020AACC);
			drawList->AddPolyline(circleArcPoints, 32, color, true, 2);

			if (ImGui::IsMouseHoveringRect(ImVec2(origin.x - circleRadius, origin.y - circleRadius), ImVec2(origin.x + circleRadius, origin.y + circleRadius)))
			{
				m_gizmoRotateHover = true;
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					m_gizmoRotateSelect = true;
					m_lastGizmoRotatePos = windowToViewport(m_mousePos);
				}
				if (m_gizmoRotateSelect && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					m_gizmoRotateSelect = false;
				}
			}
			else
			{
				m_gizmoRotateHover = false;
			}
		}
		else
		{
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
				drawList->AddRectFilled(ImVec2(endPointX.x, endPointX.y - rectSize), ImVec2(endPointX.x + 2 * rectSize, endPointX.y + rectSize), colorX);
				// Y-square	
				drawList->AddRectFilled(ImVec2(endPointY.x - rectSize, endPointY.y - 2 * rectSize), ImVec2(endPointY.x + rectSize, endPointY.y), colorY);
			}
			ImU32 colorSquare = (m_gizmoSelectSquare || m_gizmoHoverSquare) ? selectionColor : directionColor[2];
			drawList->AddRectFilled(ImVec2(origin.x + lineThickness/2.0f, origin.y - squareSize - lineThickness/2.0f), ImVec2(origin.x + squareSize + lineThickness/2.0, origin.y - lineThickness/2.0f), colorSquare);

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

			if (ImGui::IsMouseHoveringRect(ImVec2(origin.x + lineThickness / 2.0f, origin.y - squareSize - lineThickness / 2.0f), ImVec2(origin.x + squareSize + lineThickness / 2.0, origin.y - lineThickness / 2.0f)))
			{
				m_gizmoHoverSquare = true;
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					m_gizmoSelectSquare = true;
					m_lastGizmoSquarePos = windowToViewport(m_mousePos);
				}
				if (m_gizmoSelectSquare && !ImGui::IsMouseDown(ImGuiMouseButton_Left))
				{
					m_gizmoSelectSquare = false;
				}
			}
			else
			{
				m_gizmoHoverSquare = false;
			}
		}

	}

	ImGui::End(); // end "Viewport"

	UI_Toolbar(); // Pause/Play Toolbar

	ImGui::End(); // end "Dockspace demo" 

	ImGui::SFML::Render(m_game->window());

}

void EditorLayer::UI_Toolbar()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	auto* colors = ImGui::GetStyle().Colors;
	const auto& buttonHovered = colors[ImGuiCol_ButtonHovered];
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
	const auto& buttonActive = colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

	ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	bool toolbarEnabled = (bool)m_ActiveScene;

	ImVec4 tintColor = ImVec4(1, 1, 1, 1);
	if (!toolbarEnabled)
		tintColor.w = 0.5f;

	float size = ImGui::GetWindowHeight() - 4.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));

	bool hasPlayButton = true; // either edit or play...
	bool hasPauseButton = m_SceneState == SceneState::Play;

	if (hasPlayButton)
	{
		std::shared_ptr<Texture> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
		if (ImGui::ImageButton(icon->GetSFMLTexture(), sf::Vector2f(size, size), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnScenePlay();
			}
			else if (m_SceneState == SceneState::Play)
			{
				OnSceneStop();
			}
		}
	}

	if (hasPauseButton)
	{
		bool isPaused = m_ActiveScene->IsPaused();
		ImGui::SameLine();
		{
			std::shared_ptr<Texture> icon = m_IconPause;
			if (ImGui::ImageButton(icon->GetSFMLTexture(), sf::Vector2f(size, size), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				m_ActiveScene->SetPaused(!isPaused);
			}
		}

		// step button
		if (isPaused)
		{
			ImGui::SameLine();
			{
				std::shared_ptr<Texture> icon = m_IconStep;
				if (ImGui::ImageButton(icon->GetSFMLTexture(), sf::Vector2f(size, size), 0, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
				{
					m_ActiveScene->Step();
				}
			}
		}
	}

	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
	ImGui::End();

}

void EditorLayer::sDoAction(const Action& action)
{
	if (action.name() == "MOUSE_MOVE")
	{
		m_mousePos = action.pos();
		Vec2 viewportPos = windowToViewport(m_mousePos);
		Vec2 deltaPos = m_lastSceneViewPos - m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));

		if (m_SceneViewMoving)
		{
			m_SceneView.setCenter(m_SceneView.getCenter() + sf::Vector2f(deltaPos.x, deltaPos.y));
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
		else if (m_gizmoSelectSquare && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoSquarePos;
			m_lastGizmoSquarePos = viewportPos;
			if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
			{
				m_inspectedEntity.getComponent<CTransform>().pos += deltaPos;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().scale += deltaPos * m_scalingFactor;
			}
		}
		else if (m_gizmoRotateSelect && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoRotatePos;
			m_lastGizmoRotatePos = viewportPos;
			float deltaRotation = atan2(deltaPos.y, deltaPos.x) * 180.0 / 3.14;
			float newAngle = m_inspectedEntity.getComponent<CTransform>().angle + m_rotationFactor * deltaRotation;
			m_inspectedEntity.getComponent<CTransform>().angle = newAngle; 
		}
	}

	if (action.name() == "MOUSE_WHEEL_SCROLL")
	{
		if (!m_SceneViewMoving && m_altPressed)
		{
			float delta = action.pos().x;
			if (delta <= -1)
			{
				m_SceneViewZoom = std::min(2.0f, m_SceneViewZoom + 0.1f);
			}
			else if (delta >= 1)
			{
				m_SceneViewZoom = std::max(0.5f, m_SceneViewZoom - 0.1f);
			}
		}
	}

	if (action.type() == "START")
	{
		if (action.name() == "TOGGLE_GRID")
		{
			m_drawGrid = !m_drawGrid;
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
		else if (action.name() == "ROTATION_GIZMO")
		{
			m_gizmoType = GIZMO_OPERATION::ROTATE;
		}
		else if (action.name() == "DELETE")
		{
			Entity inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();
			if (inspectedEntity)
			{
				m_SceneHierarchyPanel.SetInspectedEntity({});
				m_ActiveScene->DestroyEntity(inspectedEntity);
			}
		}
		else if (action.name() == "DUPLICATE")
		{
			Entity inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();
			if (inspectedEntity)
			{
				//TODO: implement duplicate entity
				/*Entity newEntity = m_ActiveScene->AddEntity(inspectedEntity);
				m_SceneHierarchyPanel.SetInspectedEntity(newEntity);*/
			}
		}
		else if (action.name() == "LEFT_CLICK")
		{
			Vec2 viewportPos = windowToViewport(m_mousePos);
			Vec2 worldPos = m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));

			if (viewportPos.x > 0 && viewportPos.x < m_viewportSize.x && viewportPos.y > 0 && viewportPos.y < m_viewportSize.y)
			{
				Entity entity = m_ActiveScene->GetEntityIfClicked(worldPos);
				if (entity)
				{
					m_SceneHierarchyPanel.SetInspectedEntity(entity);
				}
				else
				{
					if (!(m_gizmoHoverX || m_gizmoHoverY || m_gizmoSelectX || m_gizmoSelectY || m_gizmoRotateHover || m_gizmoRotateSelect || m_gizmoSelectSquare || m_gizmoHoverSquare))
					{
						m_SceneHierarchyPanel.SetInspectedEntity({});

					}
				}
			}

			if (m_altPressed)
			{
				m_SceneViewMoving = true;
				m_lastSceneViewPos = worldPos;
			}

		}
		else if (action.name() == "QUIT")
		{
			if (m_SceneState == SceneState::Play)
				OnSceneStop();
			SaveScene();
			SaveProject();
			m_hasEnded = true;
			onEnd();
		}
		
	}
	
	if (action.type() == "END")
	{
		if (action.name() == "ALT")
		{
			m_altPressed = false;
			m_SceneViewMoving = false;
		}
		if (action.name() == "LEFT_CLICK")
		{
			if (m_altPressed)
			{
				m_SceneViewMoving = false;
			}
			if (m_gizmoSelectX)
			{
				m_gizmoSelectX = false;
			}
			if (m_gizmoSelectY)
			{
				m_gizmoSelectY = false;
			}
			if (m_gizmoRotateSelect)
			{
				m_gizmoRotateSelect = false;
			}
			if (m_gizmoSelectSquare)
			{
				m_gizmoSelectSquare = false;
			}
		}
		else if (action.name() == "PLAY_Scene")
		{
			// Scene state, Scene play
			if (m_SceneState == SceneState::Edit)
			{
				m_SceneState = SceneState::Play;
			}
			else if (m_SceneState == SceneState::Play)
				m_SceneState = SceneState::Edit;
		}
	}
}

void EditorLayer::onEnd()
{
	m_game->quit();
}