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

            "vendor/stb_image/**.h",
            "vendor/stb_image/**.cpp",

            "vendor/glm/glm/**.hpp",
            "vendor/glm/glm/**.inl",
            
            "vendor/ImGuizmo/ImGuizmo.h",
            "vendor/ImGuizmo/ImGuizmo.cpp",

            "Elysium/*.rc",
            "Elysium/resource.h",
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS",
            "GLFW_INCLUDE_NONE"
        }

        
        includedirs
        {
            "src/",

            "%{IncludeDir.yaml_cpp}",
            "%{IncludeDir.GLFW}",
            "%{IncludeDir.Glad}",
            "%{IncludeDir.ImGui}",
            "%{IncludeDir.stb_image}",
            "%{IncludeDir.glm}",
            "%{IncludeDir.ImGuizmo}"

        }
        
        links
        {
            "GLFW",
            "Glad",
            "ImGui",

            "yaml-cpp",

            "opengl32",        -- OpenGL

        }


        filter "system:windows"
            systemversion "latest"
            defines { "WINDOWS" }
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
        