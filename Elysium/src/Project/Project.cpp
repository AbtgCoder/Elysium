#include "Project.h"
#include "ProjectSerializer.h"

std::shared_ptr<Project> Project::New()
{
    s_ActiveProject = std::make_shared<Project>();
    return s_ActiveProject;
}

std::shared_ptr<Project> Project::New(const std::string& name, const std::string& location)
{
    std::shared_ptr<Project> project = std::make_shared<Project>();
    project->m_ProjectDirectory = std::filesystem::path(location).parent_path();
    auto& config = project->GetConfig();
    config.Name = name;
    config.AssetDirectory = "Assets";
    config.AssetRegistryPath = name + ".assetregistry";
    config.ScriptModulePath = "bin/" + name + ".dll";
    s_ActiveProject = project;
    std::shared_ptr<EditorAssetManager> editorAssetManager = std::make_shared<EditorAssetManager>();
    s_ActiveProject->m_AssetManager = editorAssetManager;
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
