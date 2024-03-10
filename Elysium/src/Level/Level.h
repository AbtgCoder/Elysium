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