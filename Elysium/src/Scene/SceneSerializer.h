#pragma once

#include "Scene.h"
#include <filesystem>

#include <yaml-cpp/yaml.h>

class SceneSerializer
{
public:
	SceneSerializer(const std::shared_ptr<Scene>& Scene);

	void Serialize(const std::filesystem::path& filepath);

	bool Deserialize(const std::filesystem::path& filepath);
private:
	void SerializeEntity(YAML::Emitter& out, Entity entity);
private:
	std::shared_ptr<Scene> m_Scene;
};