#pragma once

#include "Core/Layer.h"
#include "core/Logger.h"

#include "Renderer/Framebuffer.h"

#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "Renderer/EditorCamera.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/PhysicsConfigPanel.h"
#include "Panels/LoggerPanel.h"
#include "Panels/SpriteSheetEditorPanel.h"
#include "Panels/AssetManagerPanel.h"

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
	
	void OnOverlayRender();
	
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
	EditorCamera m_EditorCamera;

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
	int m_GizmoType = -1;
	float m_GizmoScaleFactor = 0.01;
	float m_GizmoRotationFactor = 0.01;

	// Debug stuff
	bool m_drawGrid = false;
	const Vec2 m_gridSize = { 64, 64 };

	Vec2 m_mousePos;

	Entity m_HoveredEntity = {};
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
	SpriteSheetEditorPanel m_SpriteSheetEditorPanel;
	AssetManagerPanel m_AssetManagerPanel;

	std::string m_DraggedInFilePath = "";
	
	std::shared_ptr<Texture2D> m_IconPlay;
	std::shared_ptr<Texture2D> m_IconPause;
	std::shared_ptr<Texture2D> m_IconStep;
	std::shared_ptr<Texture2D> m_IconStop;


};