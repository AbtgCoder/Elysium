#include "SceneImporter.h"

#include "Project/Project.h"
#include "Scene/SceneSerializer.h"

std::shared_ptr<Scene> SceneImporter::ImportScene(AssetHandle handle, const AssetMetadata& metadata)
{
	return LoadScene(Project::GetActiveAssetDirectory() / metadata.FilePath);
}

std::shared_ptr<Scene> SceneImporter::LoadScene(const std::filesystem::path& path)
{
	std::shared_ptr<Scene> scene = std::make_shared<Scene>();
	SceneSerializer serializer(scene);
	serializer.Deserialize(path);
	return scene;
}

void SceneImporter::SaveScene(std::shared_ptr<Scene> Scene, const std::filesystem::path& path)
{
	SceneSerializer serializer(Scene);
	serializer.Serialize(Project::GetActiveAssetDirectory() / path);
}
