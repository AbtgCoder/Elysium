#pragma once

#include "Asset/Asset.h"

#include "EntityManager.h"

#include <string>

class Level : public Asset
{
public:
	Level();
	Level(const std::string& name);
	~Level();

	Entity AddEntity(Entity entity);
	Entity AddEntityWithSprite(Vec2 pos,AssetHandle textureHandle);
	Entity GetEntityIfClicked(Vec2 mousePos);
	void DestroyEntity(Entity entity);

	std::vector<Entity>& GetAllPhysicsEntities();

	virtual AssetType GetType() const { return AssetType::Level; }

	std::string GetName() const { return m_Name; }

	void OnUpdateRuntime(sf::RenderTexture& renderTexture, bool drawPhysicsColliders);
	void OnUpdateEditor(sf::RenderTexture& renderTexture, bool drawPhysicsColliders);

	bool IsRunning() const { return m_IsRunning; }
	bool IsPaused() const { return m_IsPaused; }

	void SetPaused(bool paused) { m_IsPaused = paused; }
private:

	Entity m_player = {};

	// debug stuff
	sf::RectangleShape m_PhysicsRect;
	sf::ConvexShape m_PhysicsPoly;

	void RenderLevel(sf::RenderTexture& renderTexture, bool drawPhysicsColliders);
private:
	EntityManager m_entityManager;
	bool m_IsRunning = false;
	bool m_IsPaused = false;



	std::string m_Name; // TODO: Move to Asset Metadata ??

	// shader test
	sf::Shader m_Shader;

	friend class LevelSerializer;
	friend class LevelHierarchyPanel;
};