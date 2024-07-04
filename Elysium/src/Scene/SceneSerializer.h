#pragma once

#include "Scene.h"
#include <filesystem>

class SceneSerializer
{
public:
	SceneSerializer(const std::shared_ptr<Scene>& Scene);

	void Serialize(const std::filesystem::path& filepath);

	bool Deserialize(const std::filesystem::path& filepath);
private:
	std::shared_ptr<Scene> m_Scene;
};