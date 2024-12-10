#include "LoggerPanel.h"
#include "core/Logger.h"
#include "ImGui/ImGuiHelper.h"

void LoggerPanel::OnImGuiRender()
{
	if (ImGui::Begin("Logger"))
	{
		if (ImGui::Button("Clear", ImVec2(60, 28)))
		{
			Logger::ClearLogs();
		}

		ImGui::SameLine();

		if (ImGui::Button("Filter", ImVec2(60, 28)))
		{
			ImGui::OpenPopup("filter_popup");
		}

		ImGui::SameLine();

		bool isEnabled = m_LogErrors;
		if (ImGui::BeginPopup("filter_popup"))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 8));
			ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 100);

			if (isEnabled)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(97.0f / 255.0f, 0, 1.0f, 1.0f));
			}
			if (ImGui::Button("Error"))
				m_LogErrors = !m_LogErrors;
			if (isEnabled)
			{
				ImGui::PopStyleColor();
			}

			ImGui::Separator();

			isEnabled = m_LogWarnings;
			if (isEnabled)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(97.0f / 255.0f, 0, 1.0f, 1.0f));
			}
			if (ImGui::Button("Warning"))
				m_LogWarnings = !m_LogWarnings;
			if (isEnabled)
			{
				ImGui::PopStyleColor();
			}

			ImGui::Separator();

			isEnabled = m_LogDebug;
			if (isEnabled)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(97.0f / 255.0f, 0, 1.0f, 1.0f));
			}
			if (ImGui::Button("Info"))
				m_LogDebug = !m_LogDebug;
			if (isEnabled)
			{
				ImGui::PopStyleColor();
			}

			ImGui::PopStyleColor();
			ImGui::PopStyleVar(2);

			ImGui::EndPopup();
		}

		ImGui::SameLine();

		isEnabled = m_AutoScroll;
		if (isEnabled)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(97.0f / 255.0f, 0, 1.0f, 1.0f));
		}
		if (ImGui::Button("Auto Scroll", ImVec2(90, 28)))
		{
			m_AutoScroll = !m_AutoScroll;
		}
		if (isEnabled)
		{
			ImGui::PopStyleColor();
		}

		ImGui::Columns(3, 0, false);
		ImGui::SetColumnWidth(0, 100.0f); // First column
		ImGui::SetColumnWidth(1, 100.0f); // Second column
		float rowHeight = ImGui::GetTextLineHeightWithSpacing();
		for (auto& l : Logger::GetLogs())
		{
			if (l.type == LOG_TYPE::VERBOSE && !m_LogDebug)
				continue;
			if (l.type == LOG_TYPE::WARNING && !m_LogWarnings)
				continue;
			if (l.type == LOG_TYPE::CRITICAL && !m_LogErrors)
				continue;

			std::string severityTxt = "";
			if (l.type == LOG_TYPE::VERBOSE)
				severityTxt = "verbose";
			else if (l.type == LOG_TYPE::WARNING)
				severityTxt = "warning";
			else
				severityTxt = "critical";

			ImVec2 column_start = ImGui::GetCursorScreenPos();
			ImVec2 column_end = column_start;
			column_end.y += rowHeight;

			ImVec4 redColor = ImVec4(0.6f, 0.1f, 0.1f, 0.2f);
			ImVec4 yellowColor = ImVec4(0.6f, 0.6f, 0.1f, 0.2f);
			ImVec4 greenColor = ImVec4(0.59f, 0.76f, 0.47f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_Text, greenColor);
			//TODO: ImGui::draw rectangle ..
			//drawList->AddRectFilled(column_start, ImVec2(column_start.x + 100, column_end.y), ImGui::GetColorU32(ImVec4(0.59, 0.76, 0.47, 0.2)));
			const std::string timeString = " [" + l.time + "]";
			ImGui::Text(timeString.c_str());
			ImGui::PopStyleColor();

			ImGui::NextColumn();
			column_start = ImGui::GetCursorScreenPos();
			ImVec4 blueColor = ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 1.0);
			ImGui::PushStyleColor(ImGuiCol_Text, blueColor);
			//TODO: ImGui::draw rectangle ..
			//drawList->AddRectFilled(column_start, ImVec2(column_start.x + 100, column_end.y), ImGui::GetColorU32(ImVec4(98 / 255.0, 174 / 255.0, 239 / 255.0, 0.2)));
			ImGui::Text(l.logger.c_str());
			ImGui::PopStyleColor();

			ImGui::NextColumn();
			column_start = ImGui::GetCursorScreenPos();
			ImVec4 color = ImVec4(1, 1, 1, 1.0);
			ImGui::PushStyleColor(ImGuiCol_Text, color);

			if (l.type == CRITICAL)
			{
				//TODO: ImGui::draw rectangle ..
				//drawList->AddRectFilled(column_start, ImVec2(column_start.x + ImGui::GetContentRegionAvail().x, column_end.y), ImGui::GetColorU32(redColor));
			}
			else if (l.type == WARNING)
			{
				//TODO: ImGui::draw rectangle ..
				//drawList->AddRectFilled(column_start, ImVec2(column_start.x + ImGui::GetContentRegionAvail().x, column_end.y), ImGui::GetColorU32(yellowColor));
			}
			else
			{
				//TODO: ImGui::draw rectangle ..
				//drawList->AddRectFilled(column_start, ImVec2(column_start.x + ImGui::GetContentRegionAvail().x, column_end.y), ImGui::GetColorU32(ImVec4(1, 1, 1, 0.0)));
			}

			std::string displayMessage = l.message;
			if (l.count > 0)
			{
				displayMessage += " (" + std::to_string(l.count) + ")";
			}

			ImGui::TextWrapped(displayMessage.c_str());
			ImGui::PopStyleColor();

			ImGui::NextColumn();
			
			//TODO: ImGui::draw line ..
		}

		if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
		{
			ImGui::SetScrollHereY(1.0f);
		}

		ImGui::Columns(1);

	}
	ImGui::End();
}
