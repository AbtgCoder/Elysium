project "Elysium"
        location "Elysium" 
        kind "ConsoleApp"
        language "C++"
        cppdialect "C++17"

        targetdir("%{wks.location}/bin/" ..OutputDir.. "/%{prj.name}") 
        objdir("%{wks.location}/bin-int/" ..OutputDir.. "/%{prj.name}") 

        files {
            "src/**.h",
            "src/**.cpp",
            "imgui/**.h",
            "imgui/**.cpp",
        }
        --removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

        includedirs
        {
            "src/",
            "imgui/",
            "%{IncludeDir.yaml_cpp}",
        }

        links
        {
                "opengl32.lib",
                "yaml-cpp"
        }

        externalincludedirs {"../../SFML-2.5.1/include"}
        syslibdirs {"../../SFML-2.5.1/lib"}

        filter "system:windows"
            systemversion "latest"
            defines { "WINDOWS" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            runtime "Debug"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
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
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
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
            debugdir ("%{wks.location}/bin/" .. OutputDir .. "/%{prj.name}")
            links
            {
                "sfml-graphics.lib",
                "sfml-window.lib",
                "sfml-system.lib"
            }
        