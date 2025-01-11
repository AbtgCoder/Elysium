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

group "Dependencies"
   include "Elysium/vendor/GLFW"
   include "Elysium/vendor/Glad"
   include "Elysium/vendor/imgui"
   include "Elysium/vendor/yaml-cpp"
group ""

group "Core"
   include "Elysium"
group ""

group "Tools"
   include "Elysium-Editor"
group ""