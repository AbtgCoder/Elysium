#pragma once

#include "../Helper/ImGuiHelper.h"

#include "Project/Project.h"

#include <filesystem>
#include <vector>
#include <memory>

class AssetManagerPanel
{
public:
	AssetManagerPanel() = default;
	AssetManagerPanel(AssetRegistry projectAssetRegistry);

	void OnImGuiRender();

	void Refresh() {m_AssetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();}

	void Open() { m_Open = true; }
	void Close() { m_Open = false; }
private:
	bool m_Open = true;

	AssetRegistry m_AssetRegistry;
};