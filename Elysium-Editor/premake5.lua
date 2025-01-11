project "Elysium-Editor"
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"
        staticruntime "off"

        targetdir("%{wks.location}/bin/" ..outputdir.. "/%{prj.name}") 
        objdir("%{wks.location}/bin-int/" ..outputdir.. "/%{prj.name}") 

        files {
            "src/**.h",
            "src/**.cpp",
            "imgui/**.h",
            "imgui/**.cpp",
        }
        --removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

        includedirs
        {
            "%{wks.location}/Elysium/src",
            "%{wks.location}/Elysium/vendor",
            "imgui/",
            "../../SFML-2.5.1/include",

            "%{IncludeDir.glm}",

        }

        libdirs 
        {
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
            --"yaml-cpp"
        }

        --externalincludedirs {"../../SFML-2.5.1/include"}
       -- syslibdirs {"../../SFML-2.5.1/lib"}
        
        defines { "SFML_STATIC" }

        filter "system:windows"
            systemversion "latest"
            defines { "WINDOWS" }
            links { "opengl32", "winmm", "gdi32" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            runtime "Debug"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
            -- links
            -- {
            --     "sfml-graphics-d.lib",
            --     "sfml-window-d.lib",
            --     "sfml-system-d.lib"
            -- }

        filter "configurations:Release"
            defines { "RELEASE" }
            runtime "Release"
            optimize "On"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
            -- links
            -- {
            --     "sfml-graphics.lib",
            --     "sfml-window.lib",
            --     "sfml-system.lib"
            -- }


        filter "configurations:Dist"
            defines { "DIST" }
            runtime "Release"
            optimize "On"
            symbols "Off"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
            -- links
            -- {
            --     "sfml-graphics.lib",
            --     "sfml-window.lib",
            --     "sfml-system.lib"
            -- }
        