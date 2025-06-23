workspace "Sandbox Project"
   architecture "x86_64"
   startproject "Sandbox"
   configurations { "Debug", "Release", "Dist" }
   flags { "MultiProcessorCompile" }

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
EngineSrcDir = "D:/Game Development/Game_Engine_Programming/Elysium"

project "Sandbox"
        kind "SharedLib"
        language "C#"
        dotnetframework "4.7.2"

        targetdir("%{wks.location}/bin/") 
        objdir("%{wks.location}/bin-int/") 

        files {
            "Assets/**.cs",
        }

        links
        {
            "Elysium-ScriptCore"
        }


        filter "configurations:Debug"
            optimize "Off"
            symbols "Default"

        filter "configurations:Release"
            optimize "On"
            symbols "Default"

        filter "configurations:Dist"
            optimize "Full"
            symbols "Off"

group "Elysium"
    include (EngineSrcDir .. "/Elysium-ScriptCore")
group ""