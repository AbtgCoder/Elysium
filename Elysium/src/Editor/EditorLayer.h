#pragma once

#include "Core/Layer.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/PhysicsConfigPanel.h"

#include <filesystem>

enum GIZMO_OPERATION
{
	TRANSLATE,
	SCALE,
	ROTATE, // TODO: add rotation gizmo!!
};

class EditorLayer : public Layer
{
public:
	EditorLayer(Application* Application = nullptr);
	void sGUI();
	void sRender();

protected:

	std::filesystem::path m_EditorProjectPath;
	std::filesystem::path m_EditorScenePath;
	std::shared_ptr<Scene> m_ActiveScene;
	std::shared_ptr<Scene> m_EditorScene;

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

	// Scene Editor Camera and Camera Controller stuff
	sf::View m_SceneView;
	Vec2 m_SceneViewCenter;
	float m_SceneViewZoom = 1.0f;
	bool m_altPressed = false;
	Vec2 m_lastSceneViewPos;
	bool m_SceneViewMoving = false;

	// Main Rendering Viewport
	sf::RenderTexture m_rt{};
	Vec2 m_viewportSize;
	std::pair<Vec2, Vec2> m_viewportBounds;

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
	sf::Text m_gridText;
	sf::RectangleShape m_gridRect;

	Vec2 m_mousePos;
	Entity m_inspectedEntity = {};

	enum class SceneState
	{
		Edit = 0, Play = 1
	};
	SceneState m_SceneState = SceneState::Edit;

	// ImGui
	void setImGuiStyle();

	// Panels
	SceneHierarchyPanel m_SceneHierarchyPanel;
	std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;
	PhysicsConfigPanel m_PhysicsConfigPanel;
	
	std::shared_ptr<Texture> m_IconPlay;
	std::shared_ptr<Texture> m_IconPause;
	std::shared_ptr<Texture> m_IconStep;
	std::shared_ptr<Texture> m_IconStop;

	void UI_Toolbar();

	Vec2 windowToViewport(const Vec2& windowPos) const;


	void init();
	void update(float ts);
	void onEnd();
	void sDoAction(const Action& action);

};