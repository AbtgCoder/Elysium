workspace "Sandbox Project"
   architecture "x64"
   startproject "Sandbox"
   configurations { "Debug", "Release", "Dist" }

OutputDir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
EngineSrcDir = "D:/Game Development/Game_Engine_Programming/Elysium"

project "Sandbox"
        kind "SharedLib"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir("%{wks.location}/bin/" ..OutputDir.. "/%{prj.name}") 
        objdir("%{wks.location}/bin-int/" ..OutputDir.. "/%{prj.name}") 

        files {
            "Assets/**.h",
            "Assets/**.cpp",
        }

        includedirs
        {
           -- "D:/Game Development/Game_Engine_Programming/Elysium/Elysium/src"
           "Assets/",
            "" ..EngineSrcDir.. "/Elysium/src",
            "" ..EngineSrcDir.. "/Elysium/vendor",
            "../../SFML-2.5.1/include",
          --  "%{IncludeDir.yaml_cpp}",
        }

        libdirs 
        {
            "D:/Game Development/Game_Engine_Programming/Elysium/bin/Release-windows-x86_64/Elysium",
            "../../SFML-2.5.1/lib"
        }

        links
        {
            "Elysium", -- link the engine static lib
            "sfml-graphics-s",
            "sfml-window-s",
            "sfml-system-s",
            "sfml-audio-s",
            "freetype",
            "opengl32",
            "winmm",
            "gdi32",
            "openal32",
            "flac",
            "vorbis",
            "vorbisfile",
            "vorbisenc",
            "ogg"
        }

        defines { "SFML_STATIC" }

        filter "system:windows"
            systemversion "latest"
            defines { "WINDOWS" }
            links { "opengl32", "winmm", "gdi32" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            runtime "Debug"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")

        filter "configurations:Release"
            defines { "RELEASE" }
            runtime "Release"
            optimize "On"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")

        filter "configurations:Dist"
            defines { "DIST" }
            runtime "Release"
            optimize "On"
            symbols "Off"
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")