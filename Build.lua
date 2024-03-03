workspace "Elysium"
   architecture "x64"
   startproject "Elysium"
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

project "Elysium"
   location "Elysium" 
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"

   targetdir("bin/" ..OutputDir.. "/%{prj.name}") 
   objdir("bin-int/" ..OutputDir.. "/%{prj.name}") 

   files
   {
       "%{prj.name}/src/**.h",
       "%{prj.name}/src/**.cpp",
       "%{prj.name}/imgui/**.h",
       "%{prj.name}/imgui/**.cpp",
   }
   --removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

   includedirs
   {
       "%{prj.name}/src/",
       "%{prj.name}/imgui/",
       "%{IncludeDir.yaml_cpp}",
   }

   links
   {
        "opengl32.lib",
        "yaml-cpp"
   }

   externalincludedirs {"../SFML-2.5.1/include"}
   syslibdirs {"../SFML-2.5.1/lib"}

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"
       debugdir ("bin/" .. OutputDir .. "/%{prj.name}")
       links
       {
           "sfml-graphics-d.lib",
           "sfml-window-d.lib",
           "sfml-system-d.lib"
       }

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"
       debugdir ("bin/" .. OutputDir .. "/%{prj.name}")
       links
       {
           "sfml-graphics.lib",
           "sfml-window.lib",
           "sfml-system.lib"
       }


   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"
       debugdir ("bin/" .. OutputDir .. "/%{prj.name}")
       links
       {
           "sfml-graphics.lib",
           "sfml-window.lib",
           "sfml-system.lib"
       }
    
     
group "Dependencies"
    include "Elysium/vendor/yaml-cpp"
   -- include "Elysium/vendor/imgui"
   
   

