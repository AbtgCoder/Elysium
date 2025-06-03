#pragma once

#include "Asset/Asset.h"
#include "Physics/Physics.h"
#include "Physics/PhysicsWorld.h"
#include "core/UUID.h"

#include "Renderer/EditorCamera.h"

#include "EntityManager.h"

#include <string>
#include <optional>

class Scene : public Asset
{
public:
	Scene();
	Scene(const std::string& name);
	~Scene();

	static std::shared_ptr<Scene> Copy(std::shared_ptr<Scene> other);

	Entity AddEntity(const std::string& name);
	Entity AddEntityWithUUID(Elysium::UUID uuid, const std::string& name);
	Entity AddEntityWithSprite(Vec2 pos,AssetHandle textureHandle);
	//Entity DuplicateEntity(Entity entity);
	Entity DuplicateEntity(Entity entity, std::optional<Elysium::UUID> newParentID = std::nullopt);
	Entity GetEntityByUUID(Elysium::UUID id);
	Entity GetEntityByEntityID(size_t id);
	void DestroyEntity(Entity entity);

	Camera GetPrimaryCamera();
	glm::mat4 GetPrimaryCameraViewMatrix();

	bool IsEntityUUIDValid(Elysium::UUID uuid);

	std::vector<Entity>& GetAllPhysicsEntities();

	virtual AssetType GetType() const { return AssetType::Scene; }

	std::string GetName() const { return m_Name; }

	void OnRuntimeStart();
	void OnRuntimeStop();
	void UpdateTransforms();
	void OnUpdateRuntime(float dt);
	void OnUpdateEditor(EditorCamera& camera);
	void OnViewportResize(uint32_t width, uint32_t height);

	// Physics
	//void LaunchBomb(sf::RenderTexture& renderTexture);

	bool IsRunning() const { return m_IsRunning; }
	bool IsPaused() const { return m_IsPaused; }

	void SetPaused(bool paused) { m_IsPaused = paused; }

	void Step(int frames = 1);
private:

	Entity m_player = {};

	void RenderScene(EditorCamera& camera);
private:
	EntityManager m_entityManager;
	bool m_IsRunning = false;
	bool m_IsPaused = false;
	int m_StepFrames = 0;

	// Physics
	PhysicsWorld* m_PhysicsWorld = nullptr;
	Vec2 m_gravity = { 0.0f, 9.8f };
	Vec2 m_externalForce = { 5.0f, 0.0f };
	int m_velocityIterations = 6;
	int m_positionIterations = 3;
	bool m_drawPhysicsColliders = false;
	bool m_KDTreeBroadPhaseCollision = false;
	std::vector<Vec2> m_contactPoints;
	Entity m_bomb;

	std::string m_Name; // TODO: Move to Asset Metadata ??

	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	// shader test
	//sf::Shader m_Shader;

	friend class SceneSerializer;
	friend class SceneHierarchyPanel;
	friend class PhysicsConfigPanel;
};