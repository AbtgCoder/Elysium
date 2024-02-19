#include "EntityInspectorPanel.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui-SFML.h"


void EntityInspectorPanel::setInspectedEntity(std::shared_ptr<Entity>& entity)
{
	m_inspectedEntity = entity;
}

std::shared_ptr<Entity> EntityInspectorPanel::getInspectedEntity() const
{
	return m_inspectedEntity;
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

void EntityInspectorPanel::OnImGuiRender()
{
	if (ImGui::BeginChild("entity_inspector", ImGui::GetContentRegionAvail()))
	{
		if (ImGui::BeginTabBar("tab bar 2"))
		{
			if (ImGui::BeginTabItem("Entity Inspector"))
			{
				auto& tag = m_inspectedEntity->tag();
				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));
				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					//tag = std::string(buffer);
					// TODO: Ability to change tag of entity, TagComponent ??
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
					DisplayAddComponentEntry<CBoundingBox>("Box Collider 2D");
					ImGui::EndPopup();
				}

				ImGui::PopItemWidth();


				DrawComponentGUI<CTransform>("Transform", m_inspectedEntity, [](auto& component)
					{
						DrawVec2Control("Position", component.pos, 0.0f, 80.0f);
						DrawVec2Control("Scale", component.scale, 0.0f, 80.0f);
						DrawFloatControl("Angle", component.angle, 0.0f, 360.0f);
					});

				DrawComponentGUI<CAnimation>("Animation", m_inspectedEntity, [](auto& component)
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

			DrawComponentGUI<CBoundingBox>("Box Collider 2D", m_inspectedEntity, [](auto& component)
				{
					DrawVec2Control("Offset", component.offset, 0.0f, 80.0f);
					DrawVec2Control("Size", component.size, 0.0f, 80.0f);
					component.halfSize = component.size / 2;
				});

			ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::EndChild();
	}
}

template<typename T>
void EntityInspectorPanel::DisplayAddComponentEntry(const std::string& entryName)
{
	if (!m_inspectedEntity->hasComponent<T>())
	{
		if (ImGui::MenuItem(entryName.c_str()))
		{
			m_inspectedEntity->addComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}