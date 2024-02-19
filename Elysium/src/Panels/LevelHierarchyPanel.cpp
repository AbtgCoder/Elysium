#include "LevelHierarchyPanel.h"

#include "imgui.h"
#include "imgui-SFML.h"


LevelHierarchyPanel::LevelHierarchyPanel(EntityManager& em)
{
	setLevelEM(em);
}

void LevelHierarchyPanel::setLevelEM(EntityManager& em)
{
	m_levelEM = em;
	m_inspectedEntity = nullptr;
}


void LevelHierarchyPanel::setSelectedEntity(std::shared_ptr<Entity> entity)
{
	m_inspectedEntity = entity;
}

void LevelHierarchyPanel::drawEntityNode(std::shared_ptr<Entity> entity)
{
	auto& tag = entity->tag();

	ImGuiTreeNodeFlags flags = ((m_inspectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth;
	bool opened = ImGui::TreeNodeEx(reinterpret_cast<void*>(entity->id()), flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_inspectedEntity = entity;
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
		if (m_inspectedEntity == entity)
		{
			m_inspectedEntity = nullptr;
		}
	}
}


void LevelHierarchyPanel::OnImGuiRender()
{
	m_levelEM.update();
	for (auto& e : m_levelEM.getEntities())
	{
		drawEntityNode(e);
	}

	if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
	{
		m_inspectedEntity = nullptr;
	}

	// Right click on blank space
	if (ImGui::BeginPopupContextWindow(0, 1, false))
	{
		if (ImGui::MenuItem("Create Empty Entity"))
		{
			m_levelEM.addEntity("Empty Entity");
		}
		ImGui::EndPopup();
	}
}
