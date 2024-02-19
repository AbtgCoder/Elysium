#include "ContentBrowserPanel.h"

#include "imgui.h"
#include "imgui-SFML.h"

#include <iostream>

ContentBrowserPanel::ContentBrowserPanel(std::filesystem::path assetDirectory)
	: m_BaseDirectory(assetDirectory), m_CurrentDirectory(assetDirectory)
{
	if (!m_DirectoryIcon.loadFromFile("../../../resources/icons/DirectoryIcon.png"))
	{
		std::cerr << "couldnt load texture!\n";
	}
	if (!m_FileIcon.loadFromFile("../../../resources/icons/FileIcon.png"))
	{
		std::cerr << "couldnt load texture!\n";
	}
}

void ContentBrowserPanel::OnImGuiRender()
{
	if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
	{
		if (ImGui::Button("<-"))
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
	}

	static float padding = 8.0f;
	static float thumbnailSize = 64.0f;
	float cellSize = thumbnailSize + padding;

	float panelWidth = ImGui::GetContentRegionAvail().x;
	int columnCount = (int)(panelWidth / cellSize);
	if (columnCount < 1)
	{
		columnCount = 1;
	}

	ImGui::Columns(columnCount, 0, false);

	for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
	{
		const auto& path = directoryEntry.path();
		std::string filenameString = path.filename().string();
		ImGui::PushID(filenameString.c_str());
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (directoryEntry.is_directory())
		{
			ImGui::ImageButton(m_DirectoryIcon, { thumbnailSize, thumbnailSize });
		}
		else
		{
			ImGui::ImageButton(m_FileIcon, { thumbnailSize, thumbnailSize });

			/*auto it = m_assets.find(directoryEntry.path().stem().string());
			if (it != m_assets.end())
			{
				sf::Texture& texture = it->second;
				float aspectRatio = (float)(texture.getSize().y) / (float)(texture.getSize().x);
				float thumbnailHeight = thumbnailSize * aspectRatio;
				float diff = thumbnailSize - thumbnailHeight;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);
				if (ImGui::ImageButton(texture, { thumbnailSize, thumbnailHeight }))
				{
					spawnEntity(directoryEntry.path().stem().string(), texture);
				}
			}
			else
			{
				ImGui::ImageButton(m_FileIcon, { thumbnailSize, thumbnailSize });
			}*/

		}
		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Import"))
			{
				// TODO: Import assets
			}
			ImGui::EndPopup();
		}
		ImGui::PopStyleColor();

		if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (directoryEntry.is_directory())
			{
				m_CurrentDirectory /= path.filename();
			}
		}
		ImGui::TextWrapped(filenameString.c_str());

		ImGui::NextColumn();
		ImGui::PopID();
	}
	ImGui::Columns(1);

	/*ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
	ImGui::SliderFloat("Padding", &padding, 0, 32);*/

}
