#include "EditorLayer.h"

#include "Core/Application.h"
#include "core/Logger.h"

#include "core/Input.h"

#include "Renderer/RenderCommand.h"
#include "Renderer/Renderer2D.h"

#include "Physics/graham_scan.h"


#include "Asset/AssetManager.h"
#include "Asset/SceneImporter.h"
#include "Asset/TextureImporter.h"

#include "Utils/FileUtils.h"
#include "Utils/StringUtils.h"

#include <imgui/imgui.h>
#include <ImGui/imgui_internal.h>

#include "ImGuizmo.h"

#include <cmath>
#include <algorithm>
#include <glm/gtc/type_ptr.hpp>

EditorLayer::EditorLayer()
	: Layer("EditorLayer")
{
}

void EditorLayer::OnAttach()
{
	m_IconPlay = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PlayButton.png");
	m_IconPause = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/PauseButton.png");
	m_IconStep = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StepButton.png");
	m_IconStop = TextureImporter::LoadTexture2D("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/StopButton.png");
	
	// framebuffer
	FramebufferSpecification fbSpec;
	fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::Depth};
	/*
		the first attachment is the main scene color buffer
		second attachment is used to store and read entity ids 
	*/
	fbSpec.Width = 1280;
	fbSpec.Height = 720;
	m_Framebuffer = Framebuffer::Create(fbSpec);

	// camera
	m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

	//OpenProject();
	OpenProject("D:\\Game Development\\Game_Engine_Programming\\Elysium\\Sandbox Project\\Sandbox.eproject");

}

void EditorLayer::OnDetach()
{
}
 
void EditorLayer::OnUpdate(float ts)
{
	// scene viewport resize
	m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

	// frame buffer resize
	if (FramebufferSpecification spec = m_Framebuffer->GetSpecification();
		m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && // zero sized framebuffer is invalid
		(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
	{
		m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		// camera updates
		m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
	}

	// render
	// reset renderer stats
	Renderer2D::ResetStats();
	// bind framebuffer
	m_Framebuffer->Bind();

	// rendercommand -> clear
	RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	RenderCommand::Clear();

	// clean our entity ID attachment to -1
	m_Framebuffer->ClearAttachment(1, -1); 

	switch (m_SceneState)
	{
		case SceneState::Edit:
		{
			m_EditorCamera.OnUpdate(ts);

			if (m_ActiveScene)
				m_ActiveScene->OnUpdateEditor(m_EditorCamera);
			break;
		}
		case SceneState::Play:
		{
			if (m_ActiveScene)
				m_ActiveScene->OnUpdateRuntime(ts);
			break;
		}
	}

	// hovered/selected entity
	auto [mx, my] = ImGui::GetMousePos();
	mx -= m_ViewportBounds[0].x;
	my -= m_ViewportBounds[0].y;
	Vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
	my = viewportSize.y - my;
	int mouseX = (int)mx;
	int mouseY = (int)my;
	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
	{
		// read pixel data
		int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
		m_HoveredEntity = pixelData == -1 ? Entity() : m_ActiveScene->GetEntityByEntityID(pixelData);
	}

	// overlay render
	OnOverlayRender();

	// framebuffer unbind
	m_Framebuffer->Unbind();
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
	m_ViewportBounds[0] = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
	m_ViewportBounds[1] = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};
	
	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();

	Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

	uint64_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
	ImGui::Image((ImTextureID)(textureID), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

	if (m_SceneState == SceneState::Edit)
	{
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
				Entity newEntity = m_ActiveScene->AddEntityWithSprite(Vec2(0.0f, 0.0f), handle);
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
		

		// Gizmos

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

		// editor camera
		const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
		glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

		//ImGuizmo::DrawGrid(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), glm::value_ptr(glm::identity<glm::mat4>()), 100.0f);

		m_inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity();

		if (m_inspectedEntity && m_GizmoType != -1)
		{
			// entity transform
			auto& tc = m_inspectedEntity.getComponent<CTransform>();
			glm::mat4 transform = tc.GetTransform();

			// snapping
			bool snap = Input::IsKeyPressed(Key::LeftControl);
			float snapValue = 0.5; // snap to 0.5m for translation/scale
			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f; // snap to 45 degrees for rotation

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform), nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::mat4 localTransform = glm::mat4(transform);

				auto& parent = m_inspectedEntity.getComponent<CParent>();
				if (parent.HasParent)
				{
					const auto& parentTransformComponent = m_ActiveScene->GetEntityByUUID(parent.ParentID).getComponent<CTransform>();
					const glm::mat4& parentTransform = parentTransformComponent.GetTransform();
					localTransform = glm::inverse(parentTransform) * localTransform;
					/*
						since imguizmo returns a transform in global space and we want the local transform,
						we need to multiply by the inverse of the parent's global transform in order to revert
						the changes from the parent transform
					*/
				}

				// decompose local transform
				float decomposedPosition[3];
				float decomposedRotation[3];
				float decomposedScale[3];
				ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(localTransform), decomposedPosition, decomposedRotation, decomposedScale);

				tc.Translation = { decomposedPosition[0], decomposedPosition[1], decomposedPosition[2]};
				tc.Scale = { decomposedScale[0], decomposedScale[1], decomposedScale[2]};
				tc.Rotation = { decomposedRotation[0], decomposedRotation[1], decomposedRotation[2] };
			}

		}
	}

	ImGui::End(); // end "Viewport"

	UI_Toolbar(); // Pause/Play Toolbar

	ImGui::End(); // end "Dockspace demo" 

}

void EditorLayer::OnOverlayRender()
{
	if (m_SceneState == SceneState::Edit)
	{
		Renderer2D::BeginScene(m_EditorCamera);
		
		// draw inspected entity outline
		if (Entity inspectedEntity = m_SceneHierarchyPanel.GetInspectedEntity())
		{
			auto transform = inspectedEntity.getComponent<CTransform>();
			if (inspectedEntity.hasComponent<CRectangle>())
			{
				auto rect = inspectedEntity.getComponent<CRectangle>();
				Renderer2D::DrawRotatedRect({ transform.GlobalTranslation.x, transform.GlobalTranslation.y }, { rect.size.x, rect.size.y }, transform.GlobalRotation.z, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
			}

			if (inspectedEntity.hasComponent<CSpriteRenderer>())
			{
				Renderer2D::DrawRect(transform.GetTransform(), glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));
			}
		}

		Renderer2D::EndScene();
	}
}


void EditorLayer::OnEvent(Event& event)
{
	if (m_SceneState == SceneState::Edit)
		m_EditorCamera.OnEvent(event);

	EventDispatcher dispatcher(event);
	dispatcher.Dispatch<KeyPressedEvent>(std::bind(&EditorLayer::OnKeyPressed, this, std::placeholders::_1));
	dispatcher.Dispatch<MouseButtonPressedEvent>(std::bind(&EditorLayer::OnMouseButtonPressed, this, std::placeholders::_1));
	dispatcher.Dispatch<WindowDropEvent>(std::bind(&EditorLayer::OnWindowDrop, this, std::placeholders::_1));
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
{
	if (e.IsRepeat())
		return false;

	bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
	bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

	switch (e.GetKeyCode())
	{
		// shortcuts
		case Key::N:
		{
			if (control)
				NewScene();
			break;
		}
		case Key::O:
		{
			if (control)
			{
				OpenProject();
			}
			break;
		}
		case Key::S:
		{
			if (control)
			{
				if (shift)
				{
					//TODO: save scene as
				}
				else
				{
					SaveScene();
				}
			}
			break;
		}


		// scene controls
		case Key::D:
		{
			if (control)
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
			break;
		}


		// gizmos
		case Key::Q:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = -1;
			break;
		}
		case Key::W:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}
		case Key::E:
		{
			if (!ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		}
		case Key::R:
		{
			if (!control && !ImGuizmo::IsUsing())
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
	}

	return false;
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
{
	if (e.GetMouseButton() == Mouse::ButtonLeft)
	{
		if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt))
		{
			m_SceneHierarchyPanel.SetInspectedEntity(m_HoveredEntity);
		}
	}
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
		auto& cameraComponent = camera.getComponent<CCamera>();
		cameraComponent.Camera.SetProjectionType(SceneCamera::ProjectionType::Orthographic);

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
