#include "ProjectSerializer.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

ProjectSerializer::ProjectSerializer(std::shared_ptr<Project> project)
	: m_Project(project)
{
}

bool ProjectSerializer::Serialize(const std::filesystem::path& filepath)
{
	const auto& config = m_Project->GetConfig();

	YAML::Emitter out;
	{
		out << YAML::BeginMap; // Root
		out << YAML::Key << "Project" << YAML::Value;
		{
			out << YAML::BeginMap; // Project
			out << YAML::Key << "Name" << YAML::Value << config.Name;
			out << YAML::Key << "StartLevel" << YAML::Value << (uint64_t)config.StartLevel;
			out << YAML::Key << "AssetDirectory" << YAML::Value << config.AssetDirectory.string();
			out << YAML::Key << "AssetRegistryPath" << YAML::Value << config.AssetRegistryPath.string();
			out << YAML::Key << "LastOpenedLevel" << YAML::Value << (uint64_t)config.lastOpenedLevel;
			out << YAML::EndMap; // Project
		}
		out << YAML::EndMap; // Root
	}

	std::ofstream fout(filepath);
	fout << out.c_str();

	return true;
}

bool ProjectSerializer::Deserialize(const std::filesystem::path& filepath)
{
	auto& config = m_Project->GetConfig();

	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		// log error
		return false;
	}

	auto projectNode = data["Project"];
	if (!projectNode)
		return false;

	config.Name = projectNode["Name"].as<std::string>();
	config.StartLevel = projectNode["StartLevel"].as<uint64_t>();
	config.AssetDirectory = projectNode["AssetDirectory"].as<std::string>();
	if (projectNode["AssetRegistryPath"])
		config.AssetRegistryPath = projectNode["AssetRegistryPath"].as<std::string>();
	if (projectNode["LastOpenedLevel"])
		config.lastOpenedLevel = projectNode["LastOpenedLevel"].as<uint64_t>();
	return true;
}
