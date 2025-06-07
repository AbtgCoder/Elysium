#include "SpriteSheetSerializer.h"

#include "Asset/TextureImporter.h"

#include "Utils/YAMLutils.h"

#include <fstream>

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

	m_SpriteSheet->m_Texture = TextureImporter::LoadTexture2D(m_SpriteSheet->m_SourcePath);//TODO: to do or not to do this here...

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