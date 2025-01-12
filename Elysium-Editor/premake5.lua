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
        }
        --removefiles {"%{prj.name}/imgui/imgui_demo.cpp"}

        includedirs
        {
            "%{wks.location}/Elysium/src",
            "%{wks.location}/Elysium/vendor",

            "%{IncludeDir.glm}",

        }

        links
        {
            "Elysium", -- link the engine static lib
        }

        defines { "SFML_STATIC" }

        filter "system:windows"
            systemversion "latest"
            -- defines { "WINDOWS" }
           -- links { "opengl32", "winmm", "gdi32" }

        filter "configurations:Debug"
            defines { "DEBUG" }
            runtime "Debug"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

        filter "configurations:Release"
            defines { "RELEASE" }
            runtime "Release"
            optimize "On"
            symbols "On"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")

        filter "configurations:Dist"
            defines { "DIST" }
            runtime "Release"
            optimize "On"
            symbols "Off"
            debugdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
        