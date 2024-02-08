workspace "Elysium"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }

   -- Workspace-wide build options for MSVC
   filter "system:windows"
      buildoptions { "/EHsc", "/Zc:preprocessor", "/Zc:__cplusplus" }

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


project "Elysium"
   location "Elysium" 
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"

   targetdir("bin/" ..OutputDir.. "/%{prj.name}") 
   objdir("bin-int/" ..OutputDir.. "/%{prj.name}") 

   files
   {
       "%{prj.name}/src/**.h",
       "%{prj.name}/src/**.cpp",
       "%{prj.name}/imgui/**.h",
       "%{prj.name}/imgui/**.cpp",
   }
   removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

   includedirs
   {
       "%{prj.name}/imgui/",
   }

   links
   {
    "opengl32.lib"
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
    
        
    
