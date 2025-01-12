#include "EditorLayer.h"

#include "Core/Application.h"
#include "core/Logger.h"

#include "Physics/graham_scan.h"


#include "Asset/AssetManager.h"
#include "Asset/SceneImporter.h"
#include "Asset/TextureImporter.h"

#include "Utils/FileUtils.h"
#include "Utils/StringUtils.h"

#include <imgui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <cmath>
#include <algorithm>

EditorLayer::EditorLayer()
	: Layer("EditorLayer")
{
}

void EditorLayer::OnAttach()
{
	//registerAction(sf::Keyboard::G, "TOGGLE_GRID");
	//registerAction(sf::Keyboard::Escape, "QUIT");
	//registerAction(sf::Keyboard::Delete, "DELETE");
	//registerAction(sf::Keyboard::D, "DUPLICATE");
	////registerAction(sf::Keyboard::P, "PLAY_Scene");
	//registerAction(sf::Keyboard::LAlt, "ALT");
	//registerAction(sf::Keyboard::W, "TRANSLATE_GIZMO");
	//registerAction(sf::Keyboard::R, "SCALE_GIZMO");
	//registerAction(sf::Keyboard::E, "ROTATION_GIZMO");
	//registerAction(sf::Keyboard::Space, "LAUNCH_BOMB");


	m_IconPlay = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PlayButton.png");
	m_IconPause = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PauseButton.png");
	m_IconStep = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StepButton.png");
	m_IconStop = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StopButton.png");
	


	//OpenProject();
	OpenProject("D:\\Game Development\\Game_Engine_Programming\\Elysium\\Sandbox Project\\Sandbox.eproject");

}

void EditorLayer::OnDetach()
{
}
 

Vec2 EditorLayer::windowToViewport(const Vec2& windowPos) const
{
	auto viewportPos = windowPos - m_viewportBounds.first;
	return viewportPos;
}
 

void EditorLayer::OnUpdate(float ts)
{
	// scene viewport resize

	// frame buffer resize
	// camera updates

	// render
	// bind framebuffer
	// rendercommand -> clear

	switch (m_SceneState)
	{
		case SceneState::Edit:
		{
			/*m_SceneView.setSize(m_viewportSize.x, m_viewportSize.y);
			m_SceneView.zoom(m_SceneViewZoom);
			m_rt.setView(m_SceneView);
			m_rt.clear();*/
			if (m_ActiveScene)
				m_ActiveScene->OnUpdateEditor();
			break;
		}
		case SceneState::Play:
		{
			/*m_rt.clear();*/
			if (m_ActiveScene)
				m_ActiveScene->OnUpdateRuntime(ts);
			break;
		}
	}

	// hovered/selected entity

	// overlay render

	// framebuffer unbind
	
}



void EditorLayer::OnImGuiRender()
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

	bool createNewProject = false;

	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New  Project", "Ctrl+Shift+N"))
			{
				createNewProject = true;
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open Project", "Ctrl+Shift+O"))
			{
				SaveProject();
				OpenProject();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Project", "Ctrl+Shift+S"))
			{
				SaveProject();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Open Scene", "Ctrl+O"))
			{
				SaveScene(); // save active Scene
				OpenScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
			{
				NewScene();
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
			{
				SaveScene();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}

	if (createNewProject)
		ImGui::OpenPopup("new_project");

	if (ImGui::IsPopupOpen("new_project"))
		NewProject();


	m_SceneHierarchyPanel.OnImGuiRender();
	m_ContentBrowserPanel->OnImGuiRender();
	m_PhysicsConfigPanel.OnImGuiRender();
	m_LoggerPanel.OnImGuiRender();

	// Viewport 
	ImGui::Begin("Viewport");
	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_viewportBounds.first = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_viewportBounds.second = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_viewportSize = { viewportPanelSize.x, viewportPanelSize.y };
	//ImGui::Image(m_rt);

	if (m_SceneState == SceneState::Edit)
	{
#if 0
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Image"))
			{
				char* file = (char*)payload->Data;
				std::string fullPath = std::string(file, 256);
				auto relativePath = std::filesystem::relative(fullPath, Project::GetActiveAssetDirectory());
				AssetHandle handle = 0;
				if (!Project::GetActive()->GetEditorAssetManager()->AssetExistsAtFilePath(relativePath))
				{
					Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
				}
				handle = Project::GetActive()->GetEditorAssetManager()->GetAssetHandle(relativePath);
				// spawn entity with sprite renderer component
				Vec2 viewportPos = windowToViewport(m_mousePos);
				//Vec2 worldPos = m_rt.mapPixelToCoords(sf::Vector2i(viewportPos.x, viewportPos.y));
				Entity newEntity = m_ActiveScene->AddEntityWithSprite(worldPos, handle);
				newEntity.getComponent<CTag>().tag = relativePath.stem().string();
				m_SceneHierarchyPanel.SetInspectedEntity(newEntity);
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("_Scene"))
			{
				char* file = (char*)payload->Data;
				std::string fullPath = std::string(file, 256);
				auto relativePath = std::filesystem::relative(fullPath, Project::GetActiveAssetDirectory());
				AssetHandle handle = 0;
				if (!Project::GetActive()->GetEditorAssetManager()->AssetExistsAtFilePath(relativePath))
				{
					Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
				}
				handle = Project::GetActive()->GetEditorAssetManager()->GetAssetHandle(relativePath);
				SaveScene(); // TODO: maybe show a prompt/pop-up for this...
				OpenScene(handle);
			}
			ImGui::EndDragDropTarget();

		}
#endif


		// Gizmos
#if 0
		m_inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();
		if (m_inspectedEntity)
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			static const ImU32 directionColor[3] = { 0xFF715ED8, 0xFF25AA25, 0xFFCC532C }; // x, y, z direction colors
			static const ImU32 selectionColor = 0xFF20AACC;
			Vec2 ePos = m_inspectedEntity.getComponent<CTransform>().GlobalTranslation;
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
				float startAngle = fmod(m_inspectedEntity.getComponent<CTransform>().GlobalRotation + 180.0f, 360.0f);
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
#endif

	}

	ImGui::End(); // end "Viewport"

	UI_Toolbar(); // Pause/Play Toolbar

	ImGui::End(); // end "Dockspace demo" 

}

void EditorLayer::OnEvent(Event& event)
{
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
{
	return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
{
	return false;
}

bool EditorLayer::OnWindowDrop(WindowDropEvent& e)
{
	return false;
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
		std::shared_ptr<Texture2D> icon = m_SceneState == SceneState::Edit ? m_IconPlay : m_IconStop;
		if (ImGui::ImageButton("#toolbar_icon", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
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
			std::shared_ptr<Texture2D> icon = m_IconPause;
			if (ImGui::ImageButton("#toolbar_icon2", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
			{
				m_ActiveScene->SetPaused(!isPaused);
			}
		}

		// step button
		if (isPaused)
		{
			ImGui::SameLine();
			{
				std::shared_ptr<Texture2D> icon = m_IconStep;
				if (ImGui::ImageButton("#toolbar_icon3", (ImTextureID)(uint64_t)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.0f, 0.0f, 0.0f, 0.0f), tintColor) && toolbarEnabled)
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

void EditorLayer::NewProject()
{

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(1000, 700));

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(32.0f, 32.0f));
	if (ImGui::BeginPopupModal("new_project", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize))
	{

		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
			bool opened = ImGui::BeginChild("ProjectTemplates", ImVec2(ImGui::GetContentRegionAvail().x * 0.5, ImGui::GetContentRegionAvail().y), false);
			if (opened)
			{
				ImGui::PopStyleVar();
				ImGui::Text("Project Templates");
				ImGui::Dummy(ImVec2(32, 16));
			}
			if (!opened)
			{
				ImGui::PopStyleVar();
			}

			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 0.2f));
			ImGui::BeginChild("TemplateContainer", { ImGui::GetContentRegionAvail().x - 64.0f, ImGui::GetContentRegionAvail().y }, true);
			{
				ImGui::Dummy({ 4, 4 });
				//TODO: Draw Projet templates
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();

			ImGui::EndChild();
			ImGui::PopStyleColor();
		}

		ImGui::SameLine();

		{
			ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 16.0f);
			bool opened = ImGui::BeginChild("ProjectSetupView", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);
			if (opened)
			{
				ImGui::PopStyleVar();

				ImGui::Text("Project Setup");

				ImGui::Dummy(ImVec2(32, 16));

				static std::string projectName = "";
				static bool showNameEmptyWarning = false;
				static bool showPathWarning = false;
				ImGui::Text("Name");
				if (showNameEmptyWarning && projectName.empty())
				{
					ImGui::SameLine();
					ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 1.0f }, "!!");
					//TODO: tooltip...
				}

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				std::strncpy(buffer, projectName.c_str(), sizeof(buffer));
				if (ImGui::InputText("##ProjectName", buffer, sizeof(buffer)))
				{
					projectName = std::string(buffer);
				}

				ImGui::Dummy(ImVec2(32, 16));

				ImGui::Text("Location");

				static std::string location = "";
				static bool isPathValid = false;
				if ((showPathWarning && location.empty()) || (!location.empty() && !(std::filesystem::exists(location) && std::filesystem::is_directory(location))))
				{
					ImGui::SameLine();
					ImGui::TextColored({ 1.0, 0.1, 0.1, 1.0 }, "!!");
					//TODO: tooltips
				}

				ImGui::TextColored(ImVec4(1, 1, 1, 0.4), "A folder in which to create your new project");
				std::string finalLocation = "";
				static std::string projectFileName = "";

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
				char buffer2[256];
				memset(buffer2, 0, sizeof(buffer2));
				std::strncpy(buffer2, location.c_str(), sizeof(buffer2));
				if (ImGui::InputText("##ProjectLocation", buffer2, sizeof(buffer2)))
				{
					location = std::string(buffer2);
				}

				ImGui::SameLine();

				if (ImGui::Button("##folderOpen"))
				{
					std::string folderPath = WindowsFileUtils::OpenFolder();
					if (!folderPath.empty())
					{
						std::replace(folderPath.begin(), folderPath.end(), '\\', '/');
						auto splits = StringUtils::Split(folderPath, '/');
						location = "";
						for (int i = 0; i < splits.size(); i++)
						{
							location += splits[i] + "/";
						}
					}
				}

				projectFileName = StringUtils::RemoveWhiteSpace(projectName) + ".eproject";

				//finalLocation = location;
				static std::string projectParentPath = location;
				projectParentPath = location;
				projectParentPath += projectName + "/";
				finalLocation = projectParentPath + projectFileName;

				if (!location.empty() && !projectName.empty())
				{
					ImGui::TextColored(ImVec4(1, 1, 1, 0.4), "Project will be created at under:");
					ImGui::PushTextWrapPos(ImGui::GetContentRegionAvail().x);
					ImGui::TextColored(ImVec4(1, 1, 1, 0.4), finalLocation.c_str());
					ImGui::PopTextWrapPos();
				}

				ImGui::Dummy({ 1, ImGui::GetContentRegionAvail().y - 42 });

				ImGui::Dummy({ ImGui::GetContentRegionAvail().x - 230 - 16, 38 });
				ImGui::SameLine();
				if (ImGui::Button("Cancel", ImVec2(100, 38)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Create", ImVec2(120, 38)))
				{
					if (projectName.empty())
					{
						showNameEmptyWarning = true;
					}
					else
					{
						showNameEmptyWarning = false;
					}

					if (location.empty())
					{
						showPathWarning = true;
					}
					else if (std::filesystem::exists(location) && std::filesystem::is_directory(location))
					{
						showPathWarning = false;
					}

					if (!showNameEmptyWarning && !showPathWarning)
					{
						// Create Project
						const auto& projectInitialSceneDirectory = projectParentPath + "/Assets/Scenes/";
						std::filesystem::create_directories(projectInitialSceneDirectory);
						Project::New(projectName, finalLocation);
						auto projectInitialScenePath = projectInitialSceneDirectory + "Gameplay.elysium";
						auto relativePath = std::filesystem::relative(projectInitialScenePath, Project::GetActiveAssetDirectory());
						std::shared_ptr<Scene> scene = std::make_shared<Scene>();
						// Add Main Camera
						auto camera = scene->AddEntity("Main Camera");
						camera.addComponent<CCamera>();
						SceneImporter::SaveScene(scene, relativePath);
						Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
						AssetHandle sceneHandle = Project::GetActive()->GetEditorAssetManager()->GetAssetHandle(relativePath);
						auto& pConfig = Project::GetActive()->GetConfig();
						pConfig.StartScene = sceneHandle;
						pConfig.lastOpenedScene = sceneHandle;
						Project::SaveActive(finalLocation);
						Logger::Log("Created Project: " + projectFileName, "editor");
						OpenProject(finalLocation);
					}
					ImGui::CloseCurrentPopup();
				}

			}

			if (!opened)
			{
				ImGui::PopStyleVar();
			}
			ImGui::EndChild();
			ImGui::PopStyleColor();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);
}

bool EditorLayer::OpenProject()
{
	std::string projectPath = WindowsFileUtils::OpenFile("Elysium Project (*.eproject)\0*.eproject\0");
	if (projectPath.empty())
		return false;

	OpenProject(projectPath);
	return true;
}

void EditorLayer::OpenProject(const std::filesystem::path& path)
{
	if (Project::Load(path))
	{
		Logger::Log("Opening Project: " + path.filename().generic_string(), "editor");
		m_EditorProjectPath = path;
		AssetHandle lastOpenedScene = Project::GetActive()->GetConfig().lastOpenedScene;
		AssetHandle startScene = Project::GetActive()->GetConfig().StartScene;
		if (lastOpenedScene)
			OpenScene(lastOpenedScene);
		else if (startScene)
			OpenScene(startScene);
		m_ContentBrowserPanel = std::make_unique<ContentBrowserPanel>(Project::GetActive());
	}
	else
	{
		Logger::Log("Couldn't load project: " + path.filename().generic_string(), "editor", LOG_TYPE::CRITICAL);
	}
}

void EditorLayer::SaveProject()
{
	Project::SaveActive(m_EditorProjectPath);
}

void EditorLayer::NewScene()
{
	std::string path = WindowsFileUtils::SaveFile("Elysium Scene (*.elysium)\0*.elysium\0");
	if (!path.empty())
	{
		auto relativePath = std::filesystem::relative(path, Project::GetActiveAssetDirectory());
		std::shared_ptr<Scene> scene = std::make_shared<Scene>();
		// Add Main Camera
		auto camera = scene->AddEntity("Main Camera");
		camera.addComponent<CCamera>();

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
	std::string path = WindowsFileUtils::OpenFile("Elysium Scene (*.elysium)\0*.elysium\0");
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
	Logger::Log("Opening Scene: " + m_ActiveScene->GetName(), "editor");
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
	Logger::Log("Starting Runtime", "editor");
	m_ActiveScene = Scene::Copy(m_EditorScene);
	m_ActiveScene->OnRuntimeStart();
	m_SceneState = SceneState::Play;
	m_SceneHierarchyPanel.SetScene(m_ActiveScene);
	m_PhysicsConfigPanel.SetScene(m_ActiveScene);
}

void EditorLayer::OnSceneStop()
{
	Logger::Log("Stoping Runtime", "editor");
	m_ActiveScene->OnRuntimeStop();
	m_SceneState = SceneState::Edit;
	m_ActiveScene = m_EditorScene;
	m_SceneHierarchyPanel.SetScene(m_ActiveScene);
	m_PhysicsConfigPanel.SetScene(m_ActiveScene);
}


#if 0

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
				m_inspectedEntity.getComponent<CTransform>().Translation.x += deltaPos.x;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().Scale.x += m_scalingFactor * deltaPos.x;
			}
		}
		else if (m_gizmoSelectY && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoPosY;
			m_lastGizmoPosY = viewportPos;
			if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
			{
				m_inspectedEntity.getComponent<CTransform>().Translation.y += deltaPos.y;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().Scale.y += m_scalingFactor * deltaPos.y;
			}
		}
		else if (m_gizmoSelectSquare && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoSquarePos;
			m_lastGizmoSquarePos = viewportPos;
			if (m_gizmoType == GIZMO_OPERATION::TRANSLATE)
			{
				m_inspectedEntity.getComponent<CTransform>().Translation += deltaPos;
			}
			else if (m_gizmoType == GIZMO_OPERATION::SCALE)
			{
				m_inspectedEntity.getComponent<CTransform>().Scale += deltaPos * m_scalingFactor;
			}
		}
		else if (m_gizmoRotateSelect && m_inspectedEntity)
		{
			deltaPos = viewportPos - m_lastGizmoRotatePos;
			m_lastGizmoRotatePos = viewportPos;
			float deltaRotation = atan2(deltaPos.y, deltaPos.x) * 180.0 / 3.14;
			float newAngle = m_inspectedEntity.getComponent<CTransform>().Rotation + m_rotationFactor * deltaRotation;
			m_inspectedEntity.getComponent<CTransform>().Rotation = newAngle;
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
			if (m_SceneState == SceneState::Edit)
			{
				Entity inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();
				if (inspectedEntity)
				{
					Entity newEntity = m_ActiveScene->DuplicateEntity(inspectedEntity);
					m_SceneHierarchyPanel.SetInspectedEntity(newEntity);
				}
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
		if (action.name() == "LAUNCH_BOMB")
		{
			if (m_SceneState == SceneState::Play)
			{
				m_ActiveScene->LaunchBomb(m_rt);
			}
		}
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
	//TODO: Save editor settings: viewport size , zoom etc ig...
	//m_game->quit();
}

#endif