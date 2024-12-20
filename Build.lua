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


OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


IncludeDir = {}
IncludeDir["yaml_cpp"] = "%{wks.location}/Elysium/vendor/yaml-cpp/include"

group "Dependencies"
    include "Elysium/vendor/yaml-cpp"
group ""

group "Core"
   include "Elysium"
group ""

group "Tools"
   include "Elysium-Editor"
group ""