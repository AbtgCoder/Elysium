#include "ContentBrowserPanel.h"

#include "Asset/TextureImporter.h"
#include "../Helper/ImGuiHelper.h"


ContentBrowserPanel::ContentBrowserPanel(std::shared_ptr<Project> project)
	: m_Project(project), m_ThumbnailCache(std::make_shared<ThumbnailCache>(project)), m_BaseDirectory(m_Project->GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
{

	m_DirectoryIcon = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/DirectoryIcon.png");
	m_FileIcon = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/FileIcon.png");

}

void ContentBrowserPanel::OnImGuiRender()
{
	ImGui::Begin("Content Browser");

	ImVec2 avail = ImGui::GetContentRegionAvail();

	std::vector<std::filesystem::path> paths;
	std::filesystem::path currentParent = m_CurrentDirectory;
	paths.push_back(currentParent);
	while (currentParent != m_BaseDirectory && currentParent.has_parent_path())
	{
		paths.push_back(currentParent);
		currentParent = currentParent.parent_path();
	}
	paths.push_back(m_BaseDirectory);

	ImGui::BeginChild("Wrapper", avail);

	avail.y = 30;
	if (ImGui::BeginChild("Path", avail, false))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 2, 4 });
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));

		const auto buttonSize = ImVec2(26, 26);
		const auto cursorStart = ImGui::GetCursorPosX();
		if (ImGui::Button("<", buttonSize))
		{
			if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		ImGui::SameLine();

		const auto cursorEnd = ImGui::GetCursorPosX();
		const auto buttonWidth = cursorEnd - cursorStart;
		if (ImGui::Button(">", buttonSize))
		{
			// TODO...
		}

		ImGui::SameLine();
		
		const uint32_t searchBarSize = 10;
		ImGui::BeginChild("searchBar", ImVec2(searchBarSize * buttonWidth, 24));
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		std::strncpy(buffer, m_SearchQuery.c_str(), sizeof(buffer));
		if (ImGui::InputTextEx("##search", "Asset search & filter ...", buffer, sizeof(buffer), ImGui::GetContentRegionAvail(), ImGuiInputTextFlags_None))
		{
			m_SearchQuery = std::string(buffer);
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetStyle().Colors[ImGuiCol_TitleBgCollapsed]);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 4));
		ImGui::BeginChild("pathBrowser", ImVec2(ImGui::GetContentRegionAvail().x, 24));
		for (int i = paths.size() - 1; i > 0; i--)
		{
			if (i != paths.size())
				ImGui::SameLine();

			std::string pathLabel;
			if (i == paths.size() - 1)
			{
				pathLabel = "Assets";
			}
			else
			{
				pathLabel = paths[i].filename().string();
			}

			if (ImGui::Button(pathLabel.c_str()))
			{
				m_CurrentDirectory = paths[i];
			}

			ImGui::SameLine();
			ImGui::Text(">");
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleColor();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}
	ImGui::EndChild();
	
	//ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetCursorPosX(), ImGui::GetCursorPosY()), ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetCursorPosY()), IM_COL32(255, 0, 0, 255), 1.0f);

	avail = ImGui::GetContentRegionAvail();
	bool childContent = ImGui::BeginChild("Content", avail);
	ImGui::SameLine();
	if (childContent)
	{
		static float padding = 8.0f;
		static float thumbnailSize = 100.0f;
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
			bool isDirectory = directoryEntry.is_directory();
			const auto& path = directoryEntry.path();
			std::string itemStr = path.filename().string();
			std::string extension = path.extension().string();

			if (extension == ".assetregistry")
				continue;

			ImVec2 prevCursor = ImGui::GetCursorPos();
			ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();

			std::string id = std::string("##") + std::filesystem::absolute(path).generic_string();
			const bool selected = ImGui::Selectable(id.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));

			if (selected && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (isDirectory)
					m_CurrentDirectory /= path.filename();
			}

			// Drag Drop Asset
			if (!isDirectory)
			{
				if (ImGui::BeginDragDropSource())
				{
					char pathBuffer[256];
					std::strncpy(pathBuffer, std::filesystem::absolute(path).generic_string().c_str(), sizeof(pathBuffer));
					std::string dragType;
					if (extension == ".png")
					{
						dragType = "_Image";
					}
					else if (extension == ".elysium")
					{
						dragType = "_Scene";
					}
					ImGui::SetDragDropPayload(dragType.c_str(), (void*)(pathBuffer), sizeof(pathBuffer));
					ImGui::Text(itemStr.c_str());
					ImGui::EndDragDropSource();
				}
			}

			auto& imguiStyle = ImGui::GetStyle();

			ImVec2 startOffset = ImVec2(imguiStyle.FramePadding.x / 2.0f, 0);
			ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.FramePadding.y / 2.0f);
			ImU32 rectColor = IM_COL32(19, 19, 19, 255);
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(prevScreenPos.x - startOffset.x, prevScreenPos.y - startOffset.y), ImVec2(prevScreenPos.x + 100 + offsetEnd.x, prevScreenPos.y + 100 + offsetEnd.y), rectColor, 1.0f);


			std::shared_ptr<Texture> thumbnail = m_DirectoryIcon;
			if (!isDirectory)
			{
				thumbnail = m_ThumbnailCache->GetOrCreateThumbnail(path);
				if (!thumbnail)
					thumbnail = m_FileIcon;
			}

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::SetCursorPos(prevCursor);
			ImGui::Image(thumbnail->GetSFMLTexture(), { thumbnailSize, thumbnailSize });
			ImGui::PopStyleColor();


			rectColor = IM_COL32(255, 255, 255, 16);
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(prevScreenPos.x - startOffset.x, prevScreenPos.y + 100 - startOffset.y), ImVec2(prevScreenPos.x + 100 + offsetEnd.x, prevScreenPos.y + 150 + offsetEnd.y), rectColor, 1.0f);

			std::string visibleName = itemStr;
			const uint32_t MAX_CHAR_NAME = 20;
			if (itemStr.size() >= MAX_CHAR_NAME)
			{
				visibleName = std::string(visibleName.begin(), visibleName.begin() + MAX_CHAR_NAME - 3) + "...";
			}
			ImGui::TextWrapped(visibleName.c_str());


			std::string fileTypeText = "FOLDER";
			if (!isDirectory)
			{
				if (extension == ".png")
				{
					fileTypeText = "TEXTURE";
				}
				else if (extension == ".elysium")
				{
					fileTypeText = "SCENE";
				}
				else
				{
					fileTypeText = "";
				}

				if (Project::GetActive()->GetEditorAssetManager()->AssetExistsAtFilePath(std::filesystem::relative(path, Project::GetActiveAssetDirectory())))
				{
					fileTypeText = "ESM_" + fileTypeText;
				}
			}
			ImVec2 texSize = ImGui::CalcTextSize(fileTypeText.c_str());
			ImGui::SetCursorPosX(prevCursor.x + 100 + offsetEnd.x - texSize.x);
			ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());

			ImGui::TextColored({ 1.0f, 1.0f, 1.0f, 0.5f }, fileTypeText.c_str());

			ImGui::NextColumn();

		}
	}
	ImGui::Columns(1);

	// Context Menu
	if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered())
	{
		ImGui::OpenPopup("window_hover_menu");
	}

	if (ImGui::BeginPopup("window_hover_menu"))
	{
		if (ImGui::MenuItem("New Folder"))
		{
		}
		ImGui::EndPopup();
	}

	ImGui::EndChild();


	ImGui::EndChild(); //  end wrapper child
	

	ImGui::End();
}

void ContentBrowserPanel::RefreshAssetTree()
{
	return;
}
