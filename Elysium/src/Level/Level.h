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

	std::shared_ptr<Entity> AddEntity(std::shared_ptr<Entity> entity);
	std::shared_ptr<Entity> AddEntityWithSprite(Vec2 pos,AssetHandle textureHandle);
	std::shared_ptr<Entity> GetEntityIfClicked(Vec2 mousePos);
	void DestroyEntity(std::shared_ptr<Entity> entity);

	virtual AssetType GetType() const { return AssetType::Level; }

	std::string GetName() const { return m_Name; }

	void OnUpdateEditor(sf::RenderTexture& renderTexture);
private:
	void RenderLevel(sf::RenderTexture& renderTexture);
private:
	EntityManager m_entityManager;
	bool m_IsRunning = false;
	bool m_IsPaued = false;

	std::string m_Name; // TODO: Move to Asset Metadata ??

	// shader test
	sf::Shader m_Shader;

	friend class LevelSerializer;
	friend class LevelHierarchyPanel;
};