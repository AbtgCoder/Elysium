#include "ContentBrowserPanel.h"

#include "Asset/TextureImporter.h"

#include "imgui.h"
#include "imgui-SFML.h"

ContentBrowserPanel::ContentBrowserPanel(std::shared_ptr<Project> project)
	: m_Project(project), m_BaseDirectory(m_Project->GetAssetDirectory()), m_CurrentDirectory(m_BaseDirectory)
{
	m_TreeNodes.push_back(TreeNode(".", 0));

	m_DirectoryIcon = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/DirectoryIcon.png");
	m_FileIcon = TextureImporter::LoadTexture("D:/Game Development/Game_Engine_Programming/Elysium/Resources/Icons/FileIcon.png");

	RefreshAssetTree();

	m_Mode = Mode::Asset;
}

void ContentBrowserPanel::OnImGuiRender()
{
	ImGui::Begin("Content Browser");

	const char* label = m_Mode == Mode::Asset ? "Asset" : "File";
	if (ImGui::Button(label))
	{
		m_Mode = m_Mode == Mode::Asset ? Mode::FileSystem : Mode::Asset;
	}

	if (m_CurrentDirectory != std::filesystem::path(m_BaseDirectory))
	{
		if (ImGui::Button("<-")) 
		{
			m_CurrentDirectory = m_CurrentDirectory.parent_path();
		}
	}

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

	if (m_Mode == Mode::Asset)
	{
		TreeNode* node = &m_TreeNodes[0];

		auto currentDir = std::filesystem::relative(m_CurrentDirectory, Project::GetActiveAssetDirectory());
		for (const auto& p : currentDir)
		{
			if (node->Path == currentDir)
				break;

			if (node->Children.find(p) != node->Children.end())
			{
				node = &m_TreeNodes[node->Children[p]];
				continue;
			}
			else
			{
				// assert(false): cant find path
			}
		}

		for (const auto& [item, treeNodeIndex] : node->Children)
		{
			bool isDirectory = std::filesystem::is_directory(Project::GetActiveAssetDirectory() / item);
			std::string itemStr = item.generic_string();
		//	ImGui::PushID(itemStr.c_str());

			ImVec2 prevCursor = ImGui::GetCursorPos();
			ImVec2 prevScreenPos = ImGui::GetCursorScreenPos();

			std::string id = std::string("##") + (Project::GetActiveAssetDirectory() / item).generic_string();
			const bool selected = ImGui::Selectable(id.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(100, 150));

			if (selected && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (isDirectory)
					m_CurrentDirectory /= item.filename();
			}

			// Drag Drop Asset
			if (!isDirectory)
			{
				if (ImGui::BeginDragDropSource())
				{
					AssetHandle handle = m_TreeNodes[treeNodeIndex].Handle;
					ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", &handle, sizeof(AssetHandle));
					ImGui::Image(m_FileIcon->GetSFMLTexture(), { 16.0f, 16.0f });
					ImGui::SameLine();
					ImGui::Text(itemStr.c_str());
					ImGui::EndDragDropSource();
				}
			}
			
			
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::SetCursorPos(prevCursor);
			if (isDirectory)
			{
				ImGui::Image(m_DirectoryIcon->GetSFMLTexture(), { thumbnailSize, thumbnailSize });
			}
			else
			{
				ImGui::Image(m_FileIcon->GetSFMLTexture(), { thumbnailSize, thumbnailSize });
			}
			ImGui::PopStyleColor();

			
			auto& imguiStyle = ImGui::GetStyle();

			ImVec2 startOffset = ImVec2(imguiStyle.FramePadding.x / 2.0f, 0);
			ImVec2 offsetEnd = ImVec2(startOffset.x, imguiStyle.FramePadding.y / 2.0f);
			ImU32 rectColor = IM_COL32(255, 255, 255, 16);
			ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(prevScreenPos.x - startOffset.x, prevScreenPos.y + 100 - startOffset.y), ImVec2(prevScreenPos.x + 100 + offsetEnd.x, prevScreenPos.y + 150 + offsetEnd.y), rectColor, 1.0f);

			std::string visibleName = itemStr;
			const uint32_t MAX_CHAR_NAME = 20;
			if (itemStr.size() >= MAX_CHAR_NAME)
			{
				visibleName = std::string(visibleName.begin(), visibleName.begin() + MAX_CHAR_NAME - 3) + "...";
			}
			ImGui::TextWrapped(visibleName.c_str());


			ImGui::SetCursorPosY(prevCursor.y + 150 - ImGui::GetTextLineHeight());
			std::string fileTypeText = "FOLDER";
			if (!isDirectory)
			{
				std::string extension = item.extension().string();
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
			}
			ImGui::TextColored({1.0f, 1.0f, 1.0f, 0.5f}, fileTypeText.c_str());

			ImGui::NextColumn();

		//	ImGui::PopID();
		}
	}
	else
	{
		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			std::string filenameString = path.filename().string();
			ImGui::PushID(filenameString.c_str());
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			if (directoryEntry.is_directory())
			{
				ImGui::ImageButton(m_DirectoryIcon->GetSFMLTexture(), {thumbnailSize, thumbnailSize});
			}
			else
			{
				/*auto it = m_assets.find(directoryEntry.path().stem().string());
				if (it != m_assets.end())
				{
					sf::Texture& texture = it->second;
					float aspectRatio = (float)(texture.getSize().y) / (float)(texture.getSize().x);
					float thumbnailHeight = thumbnailSize * aspectRatio;
					float diff = thumbnailSize - thumbnailHeight;
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);
					ImGui::ImageButton(texture, { thumbnailSize, thumbnailHeight });
					if (ImGui::BeginDragDropSource())
					{
						ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM_TEXTURE", &directoryEntry.path().stem().string(), sizeof(std::string));
						ImGui::EndDragDropSource();
					}
				}
				else
				{*/
					ImGui::ImageButton(m_FileIcon->GetSFMLTexture(), {thumbnailSize, thumbnailSize});
				//}

				
				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Import"))
					{
						auto relativePath = std::filesystem::relative(path, Project::GetActiveAssetDirectory());
						Project::GetActive()->GetEditorAssetManager()->ImportAsset(relativePath);
						RefreshAssetTree();
					}
					ImGui::EndPopup();
				}
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
	}

	ImGui::Columns(1);

	//ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
	//ImGui::SliderFloat("Padding", &padding, 0, 32);

	ImGui::End();
}

void ContentBrowserPanel::RefreshAssetTree()
{
	const auto& assetRegistry = Project::GetActive()->GetEditorAssetManager()->GetAssetRegistry();
	for (const auto& [handle, metadata] : assetRegistry)
	{
		uint32_t currentNodeIndex = 0;
		for (const auto& p : metadata.FilePath)
		{
			auto it = m_TreeNodes[currentNodeIndex].Children.find(p.generic_string());
			if (it != m_TreeNodes[currentNodeIndex].Children.end())
			{
				currentNodeIndex = it->second;
			}
			else
			{
				// Add Node
				TreeNode newNode(p, handle);
				newNode.Parent = currentNodeIndex;
				m_TreeNodes.push_back(newNode);

				m_TreeNodes[currentNodeIndex].Children[p] = (uint32_t)m_TreeNodes.size() - 1;
				currentNodeIndex = (uint32_t)m_TreeNodes.size() - 1;
			}
		}
	}
}
