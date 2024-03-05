#include "LevelSerializer.h"

#include "Core/UUID.h"
#include "Project/Project.h"

#include <yaml-cpp/yaml.h>

#include <fstream>


namespace YAML
{
	template<>
	struct convert<Vec2>
	{
		static Node encode(const Vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}
		static bool decode(const Node& node, Vec2& rhs)
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

	template<>
	struct convert<Elysium::UUID>
	{
		static Node encode(const Elysium::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}
		static bool decode(const Node& node, Elysium::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};
}

YAML::Emitter& operator<<(YAML::Emitter& out, const Vec2& v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

LevelSerializer::LevelSerializer(const std::shared_ptr<Level>& level)
	: m_Level(level)
{
}

static void SerializeEntity(YAML::Emitter& out, std::shared_ptr<Entity> entity)
{
	out << YAML::BeginMap; // Entity
	out << YAML::Key << "Entity" << YAML::Value << entity->getComponent<CTag>().tag;
	if (entity->hasComponent<CTransform>())
	{
		out << YAML::Key << "Transform";
		out << YAML::BeginMap; // Transform Component

		auto& tc = entity->getComponent<CTransform>();
		out << YAML::Key << "Position" << YAML::Value << tc.pos;
		out << YAML::Key << "Scale" << YAML::Value << tc.scale;
		out << YAML::Key << "Angle" << YAML::Value << tc.angle;

		out << YAML::EndMap; // TransformComponent
	}
	if (entity->hasComponent<CSpriteRenderer>())
	{
		out << YAML::Key << "SpriteRenderer";
		out << YAML::BeginMap;
		
		auto& spriteRendererComponent = entity->getComponent<CSpriteRenderer>();
		out << YAML::Key << "TextureHandle" << YAML::Value << spriteRendererComponent.texture;
		out << YAML::Key << "Layer" << YAML::Value << spriteRendererComponent.layer;

		out << YAML::EndMap;
	}
	//if (entity->hasComponent<CAnimation>())
	//{
	//	out << YAML::Key << "Animation";
	//	out << YAML::BeginMap;

	//	auto& ac = entity->getComponent<CAnimation>();
	//	out << YAML::Key << "Texture" << YAML::Value << ac.animation.getName();
	//	out << YAML::Key << "Speed" << YAML::Value << ac.animSpeed;
	//	out << YAML::Key << "Frames" << YAML::Value << ac.frameCount;
	//	out << YAML::Key << "Repeatable" << YAML::Value << ac.repeat;
	//	out << YAML::Key << "Layer" << YAML::Value << ac.layer;

	//	out << YAML::EndMap;
	//}
	if (entity->hasComponent<CBoundingBox>())
	{
		out << YAML::Key << "AABB";
		out << YAML::BeginMap;

		auto& bc2d = entity->getComponent<CBoundingBox>();
		out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
		out << YAML::Key << "Size" << YAML::Value << bc2d.size;

		out << YAML::EndMap;
	}
	if (entity->hasComponent<CPolygonCollider>())
	{
		out << YAML::Key << "PolygonCollider";
		out << YAML::BeginMap;

		auto& pc2d = entity->getComponent<CPolygonCollider>();
		out << YAML::Key << "Offset" << YAML::Value << pc2d.offset;
		out << YAML::Key << "Points" << YAML::Value << YAML::BeginMap;
		out << YAML::Key << "Size" << YAML::Value << pc2d.colliderVertices.size();
		out << YAML::Key << "Elements" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		for (auto& point : pc2d.colliderVertices)
		{
			out << point;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
		out << YAML::EndMap;
	}
	if (entity->hasComponent<CGravity>())
	{
		out << YAML::Key << "Gravity";
		out << YAML::BeginMap;

		auto& gc = entity->getComponent<CGravity>();
		out << YAML::Key << "Gravity" << YAML::Value << gc.gravity;
		out << YAML::EndMap;
	}
	out << YAML::EndMap;
}

void LevelSerializer::Serialize(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Level" << YAML::Value << "Untitled";
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	for (auto entity : m_Level->m_entityManager.getEntities())
	{
		if (entity->isActive())
		{
			SerializeEntity(out, entity);
		}
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool LevelSerializer::Deserialize(const std::filesystem::path& filepath)
{
	YAML::Node data;
	try
	{
		data = YAML::LoadFile(filepath.string());
	}
	catch (YAML::ParserException e)
	{
		std::cout << "Couldnt load file: " << filepath << "\n";
		return false;
	}


	if (!data["Level"])
	{
		std::cout << "no level data\n";
		return false;
	}

	m_Level->m_Name = data["Level"].as<std::string>();

	auto entities = data["Entities"];
	if (entities)
	{
		for (auto entity : entities)
		{
			auto tag = entity["Entity"].as<std::string>();
			std::shared_ptr<Entity> deserializedEntity = m_Level->m_entityManager.addEntity(tag);
			deserializedEntity->addComponent<CTag>(tag);

			auto transformComponent = entity["Transform"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity->addComponent<CTransform>();
				tc.pos = transformComponent["Position"].as<Vec2>();
				tc.scale = transformComponent["Scale"].as<Vec2>();
				tc.angle = transformComponent["Angle"].as<float>();
			}

			auto spriteRendererComponent = entity["SpriteRenderer"];
			if (spriteRendererComponent)
			{
				auto& src = deserializedEntity->addComponent<CSpriteRenderer>();
				if (spriteRendererComponent["TextureHandle"])
				{
					src.texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();
				}
				src.layer = spriteRendererComponent["Layer"].as<int>();
			}

			auto animationComponent = entity["Animation"];
			/*if (animationComponent)
			{
				std::string textureName = animationComponent["Texture"].as<std::string>();
				size_t animationSpeed = animationComponent["Speed"].as<size_t>();
				size_t frameCount = animationComponent["Frames"].as<size_t>();
				bool repeat = animationComponent["Repeatable"].as<bool>();
				auto& ac = deserializedEntity->addComponent<CAnimation>(Animation(textureName, m_assets[textureName], frameCount, animationSpeed), repeat);
				ac.animSpeed = animationSpeed;
				ac.frameCount = frameCount;
				ac.layer = animationComponent["Layer"].as<int>();
			}*/

			auto boundingBoxComponent = entity["AABB"];
			if (boundingBoxComponent)
			{
				auto& bc2d = deserializedEntity->addComponent<CBoundingBox>();
				bc2d.offset = boundingBoxComponent["Offset"].as<Vec2>();
				bc2d.size = boundingBoxComponent["Size"].as<Vec2>();
				bc2d.halfSize = bc2d.size / 2;
			}

			auto polygonColliderComponent = entity["PolygonCollider"];
			if (polygonColliderComponent)
			{
				auto& pc2d = deserializedEntity->addComponent<CPolygonCollider>();
				pc2d.offset = polygonColliderComponent["Offset"].as<Vec2>();
				auto verticesData = polygonColliderComponent["Points"];
				auto elements = verticesData["Elements"];
				for (auto element : elements)
				{
					pc2d.colliderVertices.push_back(element.as<Vec2>());
				}
			}

			auto gravityComponent = entity["Gravity"];
			if (gravityComponent)
			{
				auto& gc = deserializedEntity->addComponent<CGravity>();
				gc.gravity = gravityComponent["Gravity"].as<float>();
			}

		}
	}

	return true;
}
