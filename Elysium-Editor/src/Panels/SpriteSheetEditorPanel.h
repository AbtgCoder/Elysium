#pragma once

#include "Animation/SpriteSheet.h"

#include "../Helper/ImGuiHelper.h"

#include <filesystem>
#include <vector>
#include <memory>

class SpriteSheetEditorPanel
{
public:
	SpriteSheetEditorPanel() = default;
	SpriteSheetEditorPanel(const std::string& texturePath);

	void OnImGuiRender();

	bool IsOpen() const { return m_Open; }
	void Open() { m_Open = true; }
	void Close() { m_Open = false; }

	void SetSpriteSheetTexture(const std::string& texturePath);

private:
	void AutoSlice();

	void AutoSliceFromAlpha();

	void SaveSpriteSheet();

private:
	std::shared_ptr<SpriteSheet> m_SpriteSheet = nullptr;
	bool m_Open = false;

	int m_SelectedIndex = 0;

	struct SliceSettings
	{
		glm::vec2 cellSize = { 32.0f, 32.0f };
		glm::vec2 offset = { 0.0f, 0.0f };
		glm::vec2 padding = { 0.0f, 0.0f };
		glm::vec2 margin = { 0.0f, 0.0f };
		bool show = false;
	};

	SliceSettings m_SliceSettings;

	//TODO: manual selection
};