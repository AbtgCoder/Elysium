#include "AnimationSerializer.h"

#include "Asset/TextureImporter.h"

#include "Utils/YAMLutils.h"

#include <fstream>


AnimationSerializer::AnimationSerializer(const std::shared_ptr<AnimationClip>& animationClip)
	: m_AnimationClip(animationClip)
{
}

void AnimationSerializer::Serialize(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	
	out << YAML::Key << "AnimationClip" << YAML::Value << m_AnimationClip->m_Name.c_str();

	out << YAML::Key << "SpriteSheetTexture" << YAML::Value << std::string(m_AnimationClip->m_SpriteSheetTexturePath.c_str());

	out << YAML::Key << "Loop" << YAML::Value << m_AnimationClip->m_Loop;

	out << YAML::Key << "Frames" << YAML::Value << YAML::BeginSeq;
	for (size_t i = 0; i < m_AnimationClip->m_Frames.size(); i++)
	{
		out << YAML::BeginMap;
		out << YAML::Key << "Frame" << YAML::Value << i;

		out << YAML::Key << "UVMin" << YAML::Value << m_AnimationClip->m_Frames[i].UVmin;
		out << YAML::Key << "UVMax" << YAML::Value << m_AnimationClip->m_Frames[i].UVMax;
		out << YAML::Key << "Duration" << YAML::Value << m_AnimationClip->m_Frames[i].Duration;

		out << YAML::EndMap;
	}
	out << YAML::EndSeq;

	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();

}

bool AnimationSerializer::Deserialize(const std::filesystem::path& filepath)
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

	if (!data["AnimationClip"])
	{
		std::cout << "no animation clip data\n";
		return false;
	}

	m_AnimationClip->m_Name = data["AnimationClip"].as<std::string>();
	m_AnimationClip->m_SpriteSheetTexturePath = data["SpriteSheetTexture"].as<std::string>();
	m_AnimationClip->m_SpriteSheetTexture = TextureImporter::LoadTexture2D(m_AnimationClip->m_SpriteSheetTexturePath);
	m_AnimationClip->m_Loop = data["Loop"].as<bool>();

	auto animFrames = data["Frames"];
	if (animFrames)
	{
		for (auto animFrame : animFrames)
		{
			m_AnimationClip->AddFrame(
				animFrame["UVMin"].as<glm::vec2>(),
				animFrame["UVMax"].as<glm::vec2>(),
				animFrame["Duration"].as<float>()
			);
		}
	}

	return true;
}