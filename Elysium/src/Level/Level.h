#pragma once

#include "Asset/Asset.h"
#include "Physics/Physics.h"

#include "EntityManager.h"

#include <string>

class Level : public Asset
{
public:
	Level();
	Level(const std::string& name);
	~Level();

	static std::shared_ptr<Level> Copy(std::shared_ptr<Level> other);

	Entity AddEntity(Entity entity);
	Entity AddEntityWithSprite(Vec2 pos,AssetHandle textureHandle);
	Entity GetEntityIfClicked(Vec2 mousePos);
	void DestroyEntity(Entity entity);

	std::vector<Entity>& GetAllPhysicsEntities();

	virtual AssetType GetType() const { return AssetType::Level; }

	std::string GetName() const { return m_Name; }

	void OnRuntimeStart();
	void OnRuntimeStop();
	void OnUpdateRuntime(sf::RenderTexture& renderTexture, bool drawPhysicsColliders, float dt);
	void OnUpdateEditor(sf::RenderTexture& renderTexture, bool drawPhysicsColliders);

	bool IsRunning() const { return m_IsRunning; }
	bool IsPaused() const { return m_IsPaused; }

	void SetPaused(bool paused) { m_IsPaused = paused; }

	void Step(int frames = 1);
private:

	Entity m_player = {};

	// debug stuff
	sf::RectangleShape m_PhysicsRect;
	sf::ConvexShape m_PhysicsPoly;
	
	// drawing shapes
	sf::CircleShape m_CircleShape;
	sf::RectangleShape m_RectangleShape;

	void RenderLevel(sf::RenderTexture& renderTexture, bool drawPhysicsColliders);
private:
	EntityManager m_entityManager;
	bool m_IsRunning = false;
	bool m_IsPaused = false;
	int m_StepFrames = 0;

	std::string m_Name; // TODO: Move to Asset Metadata ??

	// shader test
	//sf::Shader m_Shader;

	friend class LevelSerializer;
	friend class LevelHierarchyPanel;
};