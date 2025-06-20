#include "AssetManagerPanel.h"

AssetManagerPanel::AssetManagerPanel(AssetRegistry projectAssetRegistry)
	: m_AssetRegistry(projectAssetRegistry)
{
}

void AssetManagerPanel::OnImGuiRender()
{
	if (!m_Open)
		return;

	ImGui::Begin("Asset Manager", &m_Open);

	// === Filter UI ===
	static char searchBuffer[128] = "";
	static AssetType selectedType = AssetType::None;

	ImGui::Text("Filter:");
	ImGui::SameLine();
	ImGui::InputTextWithHint("##Search", "Search filename...", searchBuffer, sizeof(searchBuffer));

	ImGui::SameLine();
	ImGui::Text("Type:");
	ImGui::SameLine();

	// Create dropdown from all available AssetTypes
	static const std::vector<AssetType> allTypes = {
		AssetType::None,
		AssetType::Scene,
		AssetType::Texture2D,
		AssetType::SpriteSheet,
		AssetType::AnimationClip
	};

	const char* currentTypeStr = AssetTypeToString(selectedType).data();
	if (ImGui::BeginCombo("##AssetTypeCombo", currentTypeStr))
	{
		for (AssetType type : allTypes)
		{
			bool isSelected = (type == selectedType);
			const char* typeStr = AssetTypeToString(type).data();
			if (ImGui::Selectable(typeStr, isSelected))
				selectedType = type;

			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Separator();

	// === Asset Table ===
	if (ImGui::BeginTable("AssetTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY))
	{
		ImGui::TableSetupColumn("Handle", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 120.0f);
		ImGui::TableSetupColumn("File Path", ImGuiTableColumnFlags_WidthStretch);
		ImGui::TableHeadersRow();

		for (const auto& [handle, metadata] : m_AssetRegistry)
		{
			// === Apply Filters ===
			std::string filename = metadata.FilePath.filename().string();

			bool matchesSearch = strlen(searchBuffer) == 0 || filename.find(searchBuffer) != std::string::npos;
			bool matchesType = selectedType == AssetType::None || metadata.Type == selectedType;

			if (!(matchesSearch && matchesType))
				continue;

			// === Table Row ===
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("%llu", handle);

			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(AssetTypeToString(metadata.Type).data());

			ImGui::TableSetColumnIndex(2);
			ImGui::TextUnformatted(filename.c_str());

			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("%s", metadata.FilePath.string().c_str());
		}

		ImGui::EndTable();
	}


	ImGui::End();
}