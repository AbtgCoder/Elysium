#include "Project.h"
#include "ProjectSerializer.h"

#include "Utils/FileSystem.h"
#include "Utils/Process.h"
#include "Platform/Windows/WindowsShell.h"
#include "core/Logger.h"

#include <fstream>


std::shared_ptr<Project> Project::New(const std::string& name, const std::string& location)
{
    std::shared_ptr<Project> project = std::make_shared<Project>();
    project->m_ProjectDirectory = std::filesystem::path(location).parent_path();
    auto& config = project->GetConfig();
    config.Name = name;
    config.AssetDirectory = "Assets";
    config.AssetRegistryPath = name + ".assetregistry";
    config.ScriptModulePath = ""; //script module path should be empty when creating a new project ig..(it will be updated when we generate the solution file ig...    // "bin/" + name + ".dll";
    config.HasScriptSolution = false;
    s_ActiveProject = project;
    std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>(); 
    s_ActiveProject->m_AssetManager = editorAssetManager; // TODO: maybe this should be based on if we are in runtime or editor ig...
    return s_ActiveProject;
}

static std::string GeneratePremakeLuaContent(const std::string& projectName)
{
    std::string premakeContent =
        R"(workspace ")" + projectName + R"(")" + "\n\t"
        R"(architecture "x86_64" )" + "\n\t"
        R"(startproject ")" + projectName + R"(")" + "\n\t"
        R"(configurations { "Debug", "Release", "Dist" })" + "\n\t"
        R"(flags { "MultiProcessorCompile" })" + "\n\n"
        R"(EngineSrcDir = ")" + Elysium::FileSystem::GetEngineRootDir().generic_string() + R"(")" + "\n\n"
        R"(project ")" + projectName + R"(")" + "\n\t"
        R"(kind "SharedLib")" + "\n\t"
        R"(language "C#")" + "\n\t"
        R"(dotnetframework "4.7.2")" + "\n\n\t"
        R"(targetdir("%{wks.location}/bin/"))" + "\n\t"
        R"(objdir("%{wks.location}/bin-int/"))" + "\n\n\t"
        R"(files)" + "\n\t{\n\t\t " + R"("Assets/**.cs",)" + "\n\t}\n\n\t"
        R"(links)" + "\n\t{\n\t\t " + R"("Elysium-ScriptCore")" + "\n\t}\n\n\t"
        R"(filter "configurations:Debug")" + "\n\t\t"
        R"(optimize "Off")" + "\n\t\t"
        R"(symbols "Default")" + "\n\n\t"
        R"(filter "configurations:Release")" + "\n\t\t"
        R"(optimize "On")" + "\n\t\t"
        R"(symbols "Default")" + "\n\n\t"
        R"(filter "configurations:Dist")" + "\n\t\t"
        R"(optimize "Full")" + "\n\t\t"
        R"(symbols "Off")" + "\n\n"
        R"(group "Elysium")" + "\n\t"
        R"(include (EngineSrcDir .. "/Elysium-ScriptCore"))" + "\n"
        R"(group "")" + "\n";

    return premakeContent;
}

void Project::EnsureScriptSolution()
{
    if (m_Config.HasScriptSolution)
        return;

    if (GenerateScriptSolution())
    {
        m_Config.HasScriptSolution = true;
        m_Config.ScriptModulePath = "bin/" + m_Config.Name + ".dll";
        
        auto projectFile = m_Config.Name + ".eproject";
        SaveActive(m_ProjectDirectory / projectFile);
        Logger::Log("Script solution generated for project: " + m_Config.Name, "Project");
    }
    else
    {
        Logger::Log("Failed to generate script engine for project: " + m_Config.Name, "Project", LOG_TYPE::CRITICAL);
    }
}

bool Project::GenerateScriptSolution()
{
    // 1) write a premake5.lua 
    auto premakePath = m_ProjectDirectory / "premake5.lua";
    {
        std::ofstream out(premakePath);
        if (!out.is_open())
        {
            Logger::Log("Unable to write premake file: " + premakePath.string(), "Project", LOG_TYPE::CRITICAL);
            return false;
        }
        out << GeneratePremakeLuaContent(m_Config.Name);
    }

    // 2) run premake to generate VS solution and csproj files
    std::filesystem::path premakeExePath = Elysium::FileSystem::GetEngineRootDir() / "Vendor" / "Binaries" / "Premake" / "Windows" / "premake5.exe";
    
    if (!std::filesystem::exists(premakeExePath))
    {
        Logger::Log("Premake executable not found at: " + premakeExePath.string(), "Project", LOG_TYPE::CRITICAL);
        return false;
    }

    // run premake , target vs2022
    {
        std::wstring cmdLine = L"\"" + premakeExePath.wstring() + L"\" --file=\"" + premakePath.wstring() + L"\" vs2022";
        // run commandline process..

        auto result = Utils::Process::RunAndCapture(cmdLine, m_ProjectDirectory);

        // always print build output
        if (!result.Output.empty())
        {
            Logger::Log("Premake Output:\n" + result.Output, "Project", LOG_TYPE::VERBOSE);
        }

        if (!result.Sucess)
        {
            Logger::Log("Premake failed with exit code: " + std::to_string(result.ExitCode), "Project", LOG_TYPE::CRITICAL);
            return false;
        }
    }

    Logger::Log("Generated Script Solution successfully", "Project");

    return true;
}

bool Project::RegenerateScriptSolution()
{
    std::filesystem::path premakeExePath = Elysium::FileSystem::GetEngineRootDir() / "Vendor" / "Binaries" / "Premake" / "Windows" / "premake5.exe";
    std::filesystem::path premakeLuaPath = m_ProjectDirectory / "premake5.lua";
    std::wstring cmdLine = L"\"" + premakeExePath.wstring() + L"\" --file=\"" + premakeLuaPath.wstring() + L"\" vs2022";
    // run commandline process..

    auto result = Utils::Process::RunAndCapture(cmdLine, m_ProjectDirectory);

    // always print build output
    if (!result.Output.empty())
    {
        Logger::Log("Premake Output:\n" + result.Output, "Project", LOG_TYPE::VERBOSE);
    }

    if (!result.Sucess)
    {
        Logger::Log("Premake failed with exit code: " + std::to_string(result.ExitCode), "Project", LOG_TYPE::CRITICAL);
        return false;
    }

    Logger::Log("Generated Script Solution successfully", "Project");

    return true;
}

bool Project::BuildScriptSolution()
{
    auto slnPath = m_ProjectDirectory / (m_Config.Name + ".sln");
    if (!std::filesystem::exists(slnPath))
    {
        Logger::Log("Solution not found: " + slnPath.string(), "Project", LOG_TYPE::CRITICAL);
        return false;
    }

    // TODO: currently we assume msbuild is added to path, we should probably find it or something ig..
    std::wstring cmd =
        L"cmd.exe /C \""
        L"call \"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\Tools\\VsDevCmd.bat\""
        L" && msbuild \"" + slnPath.wstring() + L"\""
        L" /p:Configuration=Release /nologo"
        L"\"";


    auto result = Utils::Process::RunAndCapture(cmd, m_ProjectDirectory);

    // always print build output
    if (!result.Output.empty())
    {
        Logger::Log("MSBuild Output:\n" + result.Output, "MSBuild", LOG_TYPE::VERBOSE);
    }

    if (!result.Sucess)
    {
        Logger::Log("MSBuild failed with exit code: " + std::to_string(result.ExitCode), "Project", LOG_TYPE::CRITICAL);
        return false;
    }


    Logger::Log("Script solution built successfully: " + slnPath.string(), "Project");
    return true;
}

bool Project::OpenScriptSolution()
{
    auto slnPath = m_ProjectDirectory / (m_Config.Name + ".sln");

    if (!std::filesystem::exists(slnPath))
    {
        Logger::Log("Script solution not found: " + slnPath.string(), "Project", LOG_TYPE::CRITICAL);
        return false;
    }

    return Platform::OpenFileInShell(slnPath, m_ProjectDirectory);
}

std::shared_ptr<Project> Project::New()
{
    s_ActiveProject = std::make_shared<Project>();
    return s_ActiveProject;
}


std::shared_ptr<Project> Project::Load(const std::filesystem::path& path)
{
    std::shared_ptr<Project> project = std::make_shared<Project>();

    ProjectSerializer serializer(project);
    if (serializer.Deserialize(path))
    {
        project->m_ProjectDirectory = path.parent_path();
        s_ActiveProject = project;
        std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
        s_ActiveProject->m_AssetManager = editorAssetManager;
        editorAssetManager->DeserializeAssetRegistry();
        return s_ActiveProject;
    }

    return nullptr;

}

bool Project::SaveActive(const std::filesystem::path& path)
{
    ProjectSerializer serializer(s_ActiveProject);
    if (serializer.Serialize(path))
    {
        s_ActiveProject->m_ProjectDirectory = path.parent_path();
        return true;
    }
    return false;
}
