project "Elysium-ScriptCore"
        kind "SharedLib"
        language "C#"
        dotnetframework "4.7.2"

        targetdir("../Elysium-Editor/Resources/Scripts") 
        objdir("../Elysium-Editor/Resources/Intermediates") 

        files {
            "src/**.cs",
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

        