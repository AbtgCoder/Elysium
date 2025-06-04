#include "SpriteSheetSerializer.h"

#include <yaml-cpp/yaml.h>

#include <fstream>

namespace YAML
{
	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
			{
				return false;
			}
			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};
}

YAML::Emitter& operator << (YAML::Emitter& out, const glm::vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

SpriteSheetSerializer::SpriteSheetSerializer(const std::shared_ptr<SpriteSheet>& spriteSheet)
	: m_SpriteSheet(spriteSheet)
{
}

void SpriteSheetSerializer::Serialize(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "SpriteSheet" << YAML::Value << filepath.stem().string();

	out << YAML::Key << "Texture" << YAML::Value << std::string(m_SpriteSheet->m_SourcePath.c_str());

	out << YAML::Key << "SubSprites" << YAML::Value << YAML::BeginSeq;
	for (size_t i = 0; i < m_SpriteSheet->m_SubSprites.size(); i++)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Subsprite" << YAML::Value << i;

		out << YAML::Key << "Name" << YAML::Value << m_SpriteSheet->m_SubSprites[i].name;
		out << YAML::Key << "Position" << YAML::Value << m_SpriteSheet->m_SubSprites[i].position;
		out << YAML::Key << "Size" << YAML::Value << m_SpriteSheet->m_SubSprites[i].size;
		
		//TODO: uv(s) ??

		out << YAML::EndMap;
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();

}

bool SpriteSheetSerializer::Deserialize(const std::filesystem::path& filepath)
{
	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "couldn't load file: " << filepath << "\n";
		return false;
	}

	if (!data["SpriteSheet"])
	{
		std::cout << "no sprite sheet data\n";
		return false;
	}

	//TODO: spritesheet name ??
	m_SpriteSheet->m_SourcePath = data["Texture"].as<std::string>();

	auto subSprites = data["SubSprites"];
	if (subSprites)
	{
		for (auto subSprite : subSprites)
		{
			SubSprite sub;
			sub.name = subSprite["Name"].as<std::string>();
			sub.position = subSprite["Position"].as<glm::vec2>();
			sub.size = subSprite["Size"].as<glm::vec2>();
			//TODO: uvs...
			m_SpriteSheet->m_SubSprites.push_back(sub);
		}
	}

	return true;
}