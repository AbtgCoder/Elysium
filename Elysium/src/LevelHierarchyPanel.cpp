#include "LevelHierarchyPanel.h"

#include "Asset/AssetManager.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"

LevelHierarchyPanel::LevelHierarchyPanel(const std::shared_ptr<Level>& level)
{
	SetLevel(level);
}

void LevelHierarchyPanel::SetLevel(const std::shared_ptr<Level>& level)
{
	m_Level = level;
	m_InspectedEntity = nullptr;
}


static void DrawVec2Control(const std::string& label, Vec2& values, float resetValue = 0.0f, float columnWidth = 64.0f)
{
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(2, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 3.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// TODO: button styles ??
	if (ImGui::Button("X", buttonSize))
	{
		values.x = resetValue;
	}

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::SameLine();
	if (ImGui::Button("Y", buttonSize))
	{
		values.y = resetValue;
	}

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

static void DrawFloatControl(const std::string& label, float& value, float vMin = 0.0f, float vMax = 360.0f, float columnWidth = 80.0f)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImVec2 contentPos = ImGui::GetCursorPos();
	float columnWidth2 = ImGui::GetColumnWidth();
	float widgetWidth = ImGui::CalcItemWidth();
	contentPos.x += (columnWidth2 - widgetWidth) / 2;
	ImGui::SetCursorPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });
	ImGui::DragFloat("##val", &value, 0.1f, vMin, vMax, "%.2f");
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

static void DrawIntControl(const std::string& label, int& value, int vMin = 0, int vMax = 120, float columnWidth = 80.0f)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImVec2 contentPos = ImGui::GetCursorPos();
	float columnWidth2 = ImGui::GetColumnWidth();
	float widgetWidth = ImGui::CalcItemWidth();
	contentPos.x += (columnWidth2 - widgetWidth) / 2;
	ImGui::SetCursorPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });
	ImGui::DragInt("##val", &value, 0.1f, vMin, vMax);
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}


template<typename T, typename UIFunction>
static void DrawComponentGUI(const std::string& name, std::shared_ptr<Entity> entity, UIFunction uiFunction)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
	if (entity->hasComponent<T>())
	{
		auto& component = entity->getComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();
		ImGui::SameLine(contentRegionAvailable.x - lineHeight);
		if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
		{
			ImGui::OpenPopup("ComponentSettings");
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove Component"))
			{
				removeComponent = true;
			}
			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
		{
			entity->removeComponent<T>();
		}
	}
}

void LevelHierarchyPanel::OnImGuiRender()
{

	if (m_Level)
	{
		m_Level->m_entityManager.update(); // separate update function ??

		ImGui::Begin("Entity Manager");
		for (auto& e : m_Level->m_entityManager.getEntities())
		{
			DrawEntityNode(e);
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
		{
			m_InspectedEntity = nullptr;
		}

		// Right click on blank space
		if (ImGui::BeginPopupContextWindow(0, 1, false))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
			{
				auto entity = m_Level->m_entityManager.addEntity("Empty Entity");
				entity->addComponent<CTag>("Empty Entity");
			}
			ImGui::EndPopup();
		}
		ImGui::End();
	}


	ImGui::Begin("Entity Inspector");
	if (m_InspectedEntity)
	{
		auto& tag = m_InspectedEntity->getComponent<CTag>().tag;
		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
		if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
		{
			ImGui::OpenPopup("AddComponent");
		}

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CTransform>("Transform");
			DisplayAddComponentEntry<CAnimation>("Animation");
			DisplayAddComponentEntry<CGravity>("Gravity");
			if (m_InspectedEntity->hasComponent<CAnimation>())
			{
				DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D", m_InspectedEntity->getComponent<CAnimation>().animation.getSize());
				//DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D", generatePolygonColliderVertices(m_inspectedEntity));
			}
			else
			{
				DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D");
				DisplayAddComponentEntry<CPolygonCollider>("Polygon Collider 2D");
			}
			DisplayAddComponentEntry<CSpriteRenderer>("Sprite Renderer");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();


		DrawComponentGUI<CTransform>("Transform", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Position", component.pos, 0.0f, 80.0f);
				DrawVec2Control("Scale", component.scale, 0.0f, 80.0f);
				DrawFloatControl("Angle", component.angle, 0.0f, 360.0f);
			});

		DrawComponentGUI<CAnimation>("Animation", m_InspectedEntity, [](auto& component)
			{
				float imgSize = 80.0f;
				ImGui::PushID("Sprite");
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 80.0f);
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + imgSize / 2 - 7.0f);
				ImGui::Text("Sprite");
				ImGui::NextColumn();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetColumnWidth() - imgSize) / 2);
				float aspectRatio = (float)(component.animation.getSize().y) / (float)(component.animation.getSize().x);
				float imgHeight = imgSize * aspectRatio;
				float diff = imgSize - imgHeight;
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + diff);
				ImGui::Image(component.animation.getSprite(), sf::Vector2f(imgSize, imgHeight));
				ImGui::Columns(1);
				ImGui::PopID();

				int animSpeed = (int)component.animSpeed;
				DrawIntControl("Speed", animSpeed, 0, 120);
				component.animSpeed = animSpeed;

				int frameCount = (int)component.frameCount;
				DrawIntControl("Frames", frameCount, 1, 20);
				component.frameCount = frameCount;

				DrawIntControl("Layer", component.layer, -1, 10);

				ImGui::Checkbox("Repeatable", &component.repeat);
				//ImGui::Checkbox("Play Animation", &m_playAnimation);
			});

		DrawComponentGUI<CSpriteRenderer>("Sprite Renderer", m_InspectedEntity, [](auto& component)
			{
				std::string label = "None";
				bool isTextureValid = false;
				if (component.texture != 0)
				{
					if (AssetManager::IsAssetHandleValid(component.texture)
						&& AssetManager::GetAssetType(component.texture) == AssetType::Texture)
					{
						const std::filesystem::path& textureFilepath = Project::GetActive()->GetEditorAssetManager()->GetFilePath(component.texture);
						label = textureFilepath.filename().string();
						isTextureValid = true;
					}
					else
					{
						label = "Invalid";
					}
				}

				ImVec2 buttonLabelSize = ImGui::CalcTextSize(label.c_str());
				buttonLabelSize.x += 20.0f;
				float buttonLabelWidth = std::max<float>(100.0f, buttonLabelSize.x);

				ImGui::Button(label.c_str(), ImVec2(buttonLabelWidth, 0.0f));
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
					{
						AssetHandle handle = *(AssetHandle*)payload->Data;
						if (AssetManager::GetAssetType(handle) == AssetType::Texture)
						{
							component.texture = handle;
						}
						else
						{
							// log warning: wrong asset type
						}
					}
					ImGui::EndDragDropTarget();
				}

				DrawIntControl("Layer", component.layer, -1, 10);
			});

		DrawComponentGUI<CBoundingBox>("Box Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
				DrawVec2Control("Size", component.size, 0.0f, 80.0f);
				component.halfSize = component.size / 2;
			});

		DrawComponentGUI<CPolygonCollider>("Polygon Collider 2D", m_InspectedEntity, [](auto& component)
			{
				DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
				if (ImGui::CollapsingHeader("Points"))
				{
					for (size_t i = 0; i < component.colliderVertices.size(); i++)
					{
						std::string label = "Point " + std::to_string(i);
						DrawVec2Control(label, component.colliderVertices[i], 0.0f, 80.0f);
					}
				}
			});

		DrawComponentGUI<CGravity>("Gravity", m_InspectedEntity, [](auto& component)
			{
				DrawFloatControl("Gravity", component.gravity, 0.0f, 10.0f);
			});
	}

	ImGui::End();
}

template<typename T, typename... TArgs>
void LevelHierarchyPanel::DisplayAddComponentEntry(const std::string& entryName, TArgs&&... mArgs)
{
	if (!m_InspectedEntity->hasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			m_InspectedEntity->addComponent<T>(std::forward<TArgs>(mArgs)...);
			ImGui::CloseCurrentPopup();
		}
	}
}



void LevelHierarchyPanel::SetInspectedEntity(std::shared_ptr<Entity> entity)
{
	m_InspectedEntity = entity;
}

void LevelHierarchyPanel::DrawEntityNode(std::shared_ptr<Entity> entity)
{
	auto& tag = entity->getComponent<CTag>().tag;

	ImGuiTreeNodeFlags flags = ((m_InspectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity->id()), flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_InspectedEntity = entity;
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
		{
			entityDeleted = true;
		}
		ImGui::EndPopup();
	}

	if (opened)
	{
		// TODO: add entity info  ?? (id, pos, etc)
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		bool opened = ImGui::TreeNodeEx((void*)9817239, flags, tag.c_str());
		if (opened)
		{
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		entity->destroy();
		if (m_InspectedEntity == entity)
		{
			m_InspectedEntity = nullptr;
		}
	}
}
