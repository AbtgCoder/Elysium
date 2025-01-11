project "Elysium"
        location "Elysium" 
        kind "StaticLib"
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

            "vendor/stb_image/**.h",
            "vendor/stb_image/**.cpp",

            "vendor/glm/glm/**.hpp",
            "vendor/glm/glm/**.inl",


            "Elysium/*.rc",
            "Elysium/resource.h",
        }
        --removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "GLFW_INCLUDE_NONE"
        }

        
        includedirs
        {
            "src/",
            "imgui/",
            "%{IncludeDir.yaml_cpp}",

            "%{IncludeDir.GLFW}",
            "%{IncludeDir.Glad}",
            --"%{IncludeDir.ImGui}",
            "%{IncludeDir.stb_image}",
            "%{IncludeDir.glm}",


            "../../SFML-2.5.1/include"
        }
        
        -- link sfml libraries statically
        libdirs 
        {
            "../../SFML-2.5.1/lib",
        }

        links
        {
            "GLFW",
            "Glad",
            --"ImGui"

            "yaml-cpp",

            "opengl32",        -- OpenGL

            "sfml-graphics-s",
            "sfml-window-s",
            "sfml-system-s",
            "sfml-audio-s",
            "freetype",        -- Add FreeType
            "winmm",           -- Windows multimedia
            "gdi32",           -- Graphics Device Interface
            "openal32",        -- OpenAL
            "flac",            -- Audio codec
            "vorbis",          -- Audio codec
            "vorbisfile",      -- Audio codec
            "vorbisenc",       -- Audio codec
            "ogg",              -- Audio codec
        }

        -- define SFML_STATIC for static linking
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
        