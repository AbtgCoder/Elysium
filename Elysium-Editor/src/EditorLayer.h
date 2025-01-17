#pragma once

#include "Core/Layer.h"
#include "core/Logger.h"

#include "Renderer/Framebuffer.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/PhysicsConfigPanel.h"
#include "Panels/LoggerPanel.h"


#include <filesystem>

enum GIZMO_OPERATION
{
	TRANSLATE,
	SCALE,
	ROTATE,
};

class EditorLayer : public Layer
{
public:
	EditorLayer();
	~EditorLayer() { Logger::Log("editor layer destroyed", "editor layer"); }

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(float ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(Event& event) override;
private:
	// event handling
	bool OnKeyPressed(KeyPressedEvent& e);
	bool OnMouseButtonPressed(MouseButtonPressedEvent& e);
	bool OnWindowDrop(WindowDropEvent& e);
	
	void NewProject();
	bool OpenProject();
	void OpenProject(const std::filesystem::path& path);
	void SaveProject();

	void NewScene();
	void OpenScene();
	void OpenScene(AssetHandle handle);
	void SaveScene();
	void SerializeScene(std::shared_ptr<Scene> Scene, const std::filesystem::path& path);
	// TODO: implement save Scene as

	void OnScenePlay();
	void OnSceneStop();

	void UI_Toolbar();
private:
	std::filesystem::path m_EditorProjectPath;
	std::filesystem::path m_EditorScenePath;
	std::shared_ptr<Scene> m_ActiveScene;
	std::shared_ptr<Scene> m_EditorScene;

	// Scene Editor Camera and Camera Controller stuff
	Vec2 m_SceneViewCenter;
	float m_SceneViewZoom = 1.0f;
	bool m_altPressed = false;
	Vec2 m_lastSceneViewPos;
	bool m_SceneViewMoving = false;

	// Main Rendering Viewport
	std::shared_ptr<Framebuffer> m_Framebuffer;
	Vec2 m_ViewportSize = { 0.0f, 0.0f };
	Vec2 m_ViewportBounds[2];
	bool m_ViewportFocused = false, m_ViewportHovered = false;

	// Gizmo stuff
	bool m_gizmoHoverX = false;
	bool m_gizmoSelectX = false;
	Vec2 m_lastGizmoPosX;
	bool m_gizmoHoverY = false;
	bool m_gizmoSelectY = false;
	Vec2 m_lastGizmoPosY;
	float m_scalingFactor = 0.05f;
	bool m_gizmoRotateHover = false;
	bool m_gizmoRotateSelect = false;
	float m_rotationFactor = 0.05;
	Vec2 m_lastGizmoRotatePos;
	bool m_gizmoSelectSquare = false;
	bool m_gizmoHoverSquare = false;
	Vec2 m_lastGizmoSquarePos;
	int m_gizmoType = GIZMO_OPERATION::TRANSLATE;

	
	// Debug stuff
	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };

	Vec2 m_mousePos;
	Entity m_inspectedEntity = {};

	enum class SceneState
	{
		Edit = 0, Play = 1
	};
	SceneState m_SceneState = SceneState::Edit;


	// Panels
	SceneHierarchyPanel m_SceneHierarchyPanel;
	std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
	PhysicsConfigPanel m_PhysicsConfigPanel;
	LoggerPanel m_LoggerPanel;
	
	std::shared_ptr<Texture2D> m_IconPlay;
	std::shared_ptr<Texture2D> m_IconPause;
	std::shared_ptr<Texture2D> m_IconStep;
	std::shared_ptr<Texture2D> m_IconStop;


};