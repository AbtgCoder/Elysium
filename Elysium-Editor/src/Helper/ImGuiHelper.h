#pragma once

#include "Math/Vec2.h"

#include <imgui/imgui.h>
#include <ImGui/imgui_internal.h>

void DrawVec2Control(const std::string& label, Vec2& values, float resetValue = 0.0f, float columnWidth = 64.0f);
void DrawFloatControl(const std::string& label, float& value, float vMin = 0.0f, float vMax = 360.0f, float columnWidth = 80.0f);
void DrawIntControl(const std::string& label, int& value, int vMin = 0, int vMax = 120, float columnWidth = 80.0f);
