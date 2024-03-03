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

	virtual AssetType GetType() const { return AssetType::Level; }

	std::string GetName() const { return m_Name; }

	void OnUpdateEditor(sf::RenderTexture& renderTexture);
private:
	void RenderLevel(sf::RenderTexture& renderTexture);
private:
	EntityManager m_entityManager;
	bool m_IsRunning = false;
	bool m_IsPaued = false;

	std::string m_Name; // Move to Asset Metadata once we have it...

	friend class LevelSerializer;
	friend class LevelHierarchyPanel;
};