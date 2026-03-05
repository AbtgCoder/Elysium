#include "SpriteSheetEditorPanel.h"

#include "Asset/TextureImporter.h"
#include "Asset/SpriteSheetImporter.h"
#include "Project/Project.h"


#include "core/Logger.h"
#include <stb_image/stb_image.h>


#include <queue>

SpriteSheetEditorPanel::SpriteSheetEditorPanel(const std::string& texturePath)
{
	SetSpriteSheetTexture(texturePath);
}

void SpriteSheetEditorPanel::OnImGuiRender()
{
	if (!m_Open || !m_SpriteSheet->m_Texture)
	{
		return;
	}

	ImGui::Begin("Sprite Sheet Editor", &m_Open);

	//ImGui::Text("Texture: %s", std::filesystem::path(m_SpriteSheet.m_SourcePath).filename().string().c_str());

	float sidebarWidth = 300.0f;
	float topBarHeight = 40.0f;

	// Top Toolbar
	if (ImGui::BeginChild("TopBar", ImVec2(0, topBarHeight), false, ImGuiWindowFlags_NoScrollbar))
	{

		if (ImGui::Button("Auto Slice"))
		{
			m_SliceSettings.show = !m_SliceSettings.show; // toggle visibility
		}

		ImGui::SameLine();

		if (ImGui::Button("Auto Slice (smart)"))
		{
			AutoSliceFromAlpha();
		}

		ImGui::SameLine();

		if (ImGui::Button("Save spritesheet"))
		{
			SaveSpriteSheet();
		}

		ImGui::SameLine();
		ImGui::Text("Sub-sprites: %zu", m_SpriteSheet->m_SubSprites.size());
	}
	ImGui::EndChild();

	if (m_SliceSettings.show)
	{
		ImGui::BeginChild("SliceSettingsPanel", ImVec2(300, 150), true, ImGuiWindowFlags_AlwaysUseWindowPadding);

		ImGui::Text("Slice Settings");
		ImGui::Separator();

		ImGui::DragFloat2("Cell Size", glm::value_ptr(m_SliceSettings.cellSize), 1.0f, 1.0f);
		ImGui::DragFloat2("Offset", glm::value_ptr(m_SliceSettings.offset), 1.0f, 0.0f);
		ImGui::DragFloat2("Padding", glm::value_ptr(m_SliceSettings.padding), 1.0f, 0.0f);
		ImGui::DragFloat2("Margin", glm::value_ptr(m_SliceSettings.margin), 1.0f, 0.0f);

		if (ImGui::Button("Apply Slice"))
		{
			AutoSlice();
			m_SliceSettings.show = false;
		}

		ImGui::EndChild();
	}

	// split: left->sprite preview | right->subsprite info

	float panelWidth = ImGui::GetContentRegionAvail().x;
	float spacing = 16.0f;

	ImGui::Separator();
	

	ImGui::BeginChild("leftpanel", ImVec2(panelWidth - sidebarWidth - spacing, 0), false);

	// left column: sprite sheet texture preview

	ImVec2 contentSize = ImGui::GetContentRegionAvail();
	float previewSize = std::min(contentSize.x, contentSize.y) * 0.75;
	float aspectRatio = (float)m_SpriteSheet->m_Texture->GetWidth() / (float)m_SpriteSheet->m_Texture->GetHeight();
	ImVec2 imageSize = { previewSize, previewSize / aspectRatio };
	if (imageSize.y > previewSize)
	{
		imageSize.y = previewSize;
		imageSize.x = previewSize * aspectRatio;
	}
	ImVec2 contentStart = ImGui::GetCursorScreenPos();
	ImVec2 centerPos = {
		contentStart.x + (contentSize.x - imageSize.x) * 0.5f,
		contentStart.y + (contentSize.y - imageSize.y) * 0.5f,
	};
	ImGui::SetCursorScreenPos(centerPos);
	ImVec2 imageTopLeft = centerPos;
	ImGui::Image((ImTextureID)m_SpriteSheet->m_Texture->GetRendererID(), imageSize, { 0, 1 }, {1, 0});

	// draw sub-sprite rect overlays
	for (size_t i = 0; i < m_SpriteSheet->m_SubSprites.size(); i++)
	{
		const SubSprite& sprite = m_SpriteSheet->m_SubSprites[i];

		ImVec2 topLeft =
		{
			imageTopLeft.x + sprite.position.x / m_SpriteSheet->m_Texture->GetWidth() * imageSize.x,
			imageTopLeft.y + sprite.position.y / m_SpriteSheet->m_Texture->GetHeight() * imageSize.y,
		};

		ImVec2 bottomRight =
		{
			topLeft.x + sprite.size.x / m_SpriteSheet->m_Texture->GetWidth() * imageSize.x,
			topLeft.y + sprite.size.y / m_SpriteSheet->m_Texture->GetHeight() * imageSize.y,
		};

		ImU32 color = (i == m_SelectedIndex) ? IM_COL32(0, 100, 255, 255) : IM_COL32(255, 255, 255, 80);
		ImGui::GetWindowDrawList()->AddRect(topLeft, bottomRight, color, 0.0f, 0, 2.0f);
	}

	// select by clicking inside a sub-sprite
	if (ImGui::IsItemClicked())
	{
		ImVec2 clickPos = ImVec2(ImGui::GetMousePos().x - imageTopLeft.x, ImGui::GetMousePos().y - imageTopLeft.y);
		glm::vec2 texPos = {
			clickPos.x / imageSize.x * m_SpriteSheet->m_Texture->GetWidth(),
			clickPos.y / imageSize.y * m_SpriteSheet->m_Texture->GetHeight()
		};
		for (size_t i = 0; i < m_SpriteSheet->m_SubSprites.size(); i++)
		{
			const SubSprite& s = m_SpriteSheet->m_SubSprites[i];
			if (texPos.x >= s.position.x && texPos.x <= s.position.x + s.size.x &&
				texPos.y >= s.position.y && texPos.y <= s.position.y + s.size.y
				)
			{
				m_SelectedIndex = i;
				break;
			}
		}
	}

	ImGui::EndChild();

	ImGui::SameLine();


	ImGui::BeginChild("RightPanel", ImVec2(0, 0), false);

	// right column: subsprite inspector
	if (m_SelectedIndex >= 0 && m_SelectedIndex < m_SpriteSheet->m_SubSprites.size())
	{
		auto& selected = m_SpriteSheet->m_SubSprites[m_SelectedIndex];

		ImGui::Text("Subsprite Properties");
		
		auto& name = selected.name;
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), name.c_str(), sizeof(buffer));
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			name = std::string(buffer);
		}

		ImGui::DragFloat2("Position", glm::value_ptr(selected.position), 1.0f, 0.0f, (float)m_SpriteSheet->m_Texture->GetWidth());
		ImGui::DragFloat2("Size", glm::value_ptr(selected.size), 1.0f, 1.0f, (float)m_SpriteSheet->m_Texture->GetWidth());

		// read-only uvs ??
	}
	ImGui::EndChild();


	ImGui::End();
}

void SpriteSheetEditorPanel::SetSpriteSheetTexture(const std::string& texturePath)
{
	if (m_SpriteSheet)
	{
		m_SpriteSheet.reset();
	}

	m_SpriteSheet = std::make_shared<SpriteSheet>();
	m_SpriteSheet->m_SourcePath = std::filesystem::relative(std::filesystem::path(texturePath), Project::GetActiveAssetDirectory()).string();
	m_SpriteSheet->m_Texture = TextureImporter::LoadTexture2D(texturePath);
	m_SpriteSheet->m_SubSprites.clear();

	SubSprite sub;
	sub.name = std::filesystem::path(texturePath).stem().string();
	sub.position = { 0, 0 };
	sub.size = { (float)m_SpriteSheet->m_Texture->GetWidth(), (float)m_SpriteSheet->m_Texture->GetHeight() };

	m_SpriteSheet->m_SubSprites.push_back(sub);
}

void SpriteSheetEditorPanel::AutoSlice()
{
	m_SpriteSheet->m_SubSprites.clear();

	std::string baseName = std::filesystem::path(m_SpriteSheet->m_SourcePath).stem().string();

	int texWidth = (int)m_SpriteSheet->m_Texture->GetWidth();
	int texHeight = (int)m_SpriteSheet->m_Texture->GetHeight();

	glm::vec2 cursor = m_SliceSettings.margin + m_SliceSettings.offset;

	int index = 0;
	for (float y = cursor.y; y + m_SliceSettings.cellSize.y <= texHeight - m_SliceSettings.margin.y; y += m_SliceSettings.cellSize.y + m_SliceSettings.padding.y)
	{
		for (float x = cursor.x; x + m_SliceSettings.cellSize.x <= texWidth - m_SliceSettings.margin.x; x += m_SliceSettings.cellSize.x + m_SliceSettings.padding.x)
		{
			SubSprite sprite;
			sprite.name = baseName + "_" + std::to_string(index++);
			sprite.position = { x, y };
			sprite.size = m_SliceSettings.cellSize;

			//TODO: calculate uv's

			m_SpriteSheet->m_SubSprites.push_back(sprite);
		}
	}

}

//TODO: optimize this ?? , store pixel data ?? here or in texture itself ??
void SpriteSheetEditorPanel::AutoSliceFromAlpha()
{
	struct Pixel
	{
		uint8_t r, g, b, a;
	};

	int width, height, channels;
	stbi_set_flip_vertically_on_load(0);
	stbi_uc* pixelData = stbi_load(m_SpriteSheet->m_SourcePath.c_str(), &width, &height, &channels, STBI_rgb_alpha);
	std::vector<Pixel> pixels(width* height);
	memcpy(pixels.data(), pixelData, width* height * 4);
	stbi_image_free(pixelData);

	m_SpriteSheet->m_SubSprites.clear();

	int alphaThreshold = 10;
	int padding = 2;
	int minWidth = 4;
	int minHeight = 4;


	std::vector<bool> visited(width* height, false);

	auto isOpaque = [&](int x, int y) -> bool {
		return pixels[y * width + x].a > alphaThreshold;
		};
	auto isValid = [&](int x, int y) -> bool {
		return x >= 0 && y >= 0 && x < width && y < height;
		};

	std::string baseName = std::filesystem::path(m_SpriteSheet->m_SourcePath).stem().string();
	int spriteIndex = 0;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			if (visited[y * width + x] || !isOpaque(x, y))
				continue;

			visited[y * width + x] = true;


			// start flood fill
			int minX = x, maxX = x, minY = y, maxY = y;
			std::queue<std::pair<int, int>> q;
			q.emplace(x, y);
			
			while (!q.empty())
			{
				auto [cx, cy] = q.front(); q.pop();
				minX = std::min(minX, cx); maxX = std::max(maxX, cx);
				minY = std::min(minY, cy); maxY = std::max(maxY, cy);

				for (int dy = -1; dy <= 1; dy++)
				{
					for (int dx = -1; dx <= 1; dx++)
					{
						int nx = cx + dx, ny = cy + dy;
						if (isValid(nx, ny) && !visited[ny * width + nx] && isOpaque(nx, ny)) // upload all nearby opaque pixels to the queue...
						{
							visited[ny * width + nx] = true;
							q.emplace(nx, ny);
						}
					}
				}
			}
			
			minX = std::max(0, minX - padding);
			minY = std::max(0, minY - padding);
			maxX = std::min(width - 1, maxX + padding);
			maxY = std::min(height - 1, maxY + padding);

			int spriteW = maxX - minX + 1;
			int spriteH = maxY - minY + 1;

			if (spriteW < minWidth || spriteH < minHeight)
				continue;

			SubSprite sprite;
			sprite.name = baseName + "_" + std::to_string(spriteIndex++);
			sprite.position = { minX, minY };
			sprite.size = { spriteW, spriteH };
			m_SpriteSheet->m_SubSprites.push_back(sprite);
		}
	}
}

void SpriteSheetEditorPanel::SaveSpriteSheet()
{
	auto projectAssetDir = Project::GetActiveAssetDirectory();
	auto spriteSheetDir = projectAssetDir / "Spritesheets";

	// Ensure the directory exists
	std::filesystem::create_directories(spriteSheetDir);

	std::string spriteSheetPath = (spriteSheetDir / std::filesystem::path(m_SpriteSheet->m_SourcePath).stem()).string() + ".esmspritesheet";

	auto relativePath = std::filesystem::relative(spriteSheetPath, Project::GetActiveAssetDirectory());

	SpriteSheetImporter::SaveSpriteSheet(m_SpriteSheet, relativePath);
	Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);

	Logger::Log("Created spritesheet !!", "editor");

	m_Open = false;
}
