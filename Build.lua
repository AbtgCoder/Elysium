workspace "Elysium"
   architecture "x64"
   startproject "Elysium-Editor"
   configurations { "Debug", "Release", "Dist" }

   flags
	{
		"MultiProcessorCompile"
	}

   -- Workspace-wide build options for MSVC
  -- filter "system:windows"
     -- buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


IncludeDir = {}
IncludeDir["yaml_cpp"] = "%{wks.location}/Elysium/vendor/yaml-cpp/include"
IncludeDir["GLFW"] = "%{wks.location}/Elysium/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/Elysium/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/Elysium/vendor/imgui"
IncludeDir["stb_image"] = "%{wks.location}/Elysium/vendor/stb_image"
IncludeDir["glm"] = "%{wks.location}/Elysium/vendor/glm"
IncludeDir["ImGuizmo"] = "%{wks.location}/Elysium/vendor/ImGuizmo"
IncludeDir["mono"] = "%{wks.location}/Elysium/vendor/mono/include"

LibraryDir = {}
LibraryDir["mono"] = "%{wks.location}/Elysium/vendor/mono/lib/%{cfg.buildcfg}"

Library = {}
Library["mono"] = "%{LibraryDir.mono}/libmono-static-sgen.lib"

-- Windows
Library["WinSock"] = "Ws2_32.lib"
Library["Winmm"] = "Winmm.lib"
Library["WinVersion"] = "Version.lib"
Library["BCrypt"] = "Bcrypt.lib"

group "Dependencies"
   include "Elysium/vendor/GLFW"
   include "Elysium/vendor/Glad"
   include "Elysium/vendor/imgui"
   include "Elysium/vendor/yaml-cpp"
group ""

group "Core"
   include "Elysium"
   include "Elysium-ScriptCore"
group ""

group "Tools"
   include "Elysium-Editor"
group ""