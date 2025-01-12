#include "ImGuiHelper.h"



void DrawVec2Control(const std::string& label, Vec2& values, float resetValue, float columnWidth)
{
	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
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

void DrawFloatControl(const std::string& label, float& value, float vMin, float vMax, float columnWidth)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImVec2 contentPos = ImGui::GetCursorPos();
	float columnWidth2 = ImGui::GetColumnWidth();
	float widgetWidth = ImGui::CalcItemWidth();
	contentPos.x += (columnWidth2 - widgetWidth) / 2;
	ImGui::SetCursorPos(contentPos);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 5, 5 });
	ImGui::DragFloat("##val", &value, 0.01f, vMin, vMax, "%.2f");
	ImGui::PopStyleVar();

	ImGui::Columns(1);
	ImGui::PopID();
}

void DrawIntControl(const std::string& label, int& value, int vMin, int vMax, float columnWidth)
{
	ImGui::PushID(label.c_str());
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 7.0f);
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
