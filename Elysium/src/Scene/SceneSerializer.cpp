#include "SceneSerializer.h"

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

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& Scene)
	: m_Scene(Scene)
{
}

static void SerializeEntity(YAML::Emitter& out, Entity entity)
{
	out << YAML::BeginMap; // Entity
	out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<CTag>().tag;
	if (entity.hasComponent<CTransform>())
	{
		out << YAML::Key << "Transform";
		out << YAML::BeginMap; // Transform Component

		auto& tc = entity.getComponent<CTransform>();
		out << YAML::Key << "Position" << YAML::Value << tc.pos;
		out << YAML::Key << "Scale" << YAML::Value << tc.scale;
		out << YAML::Key << "Angle" << YAML::Value << tc.angle;
		out << YAML::Key << "Velocity" << YAML::Value << tc.velocity;
		out << YAML::Key << "AngularVelocity" << YAML::Value << tc.angularVelocity;

		out << YAML::EndMap; // TransformComponent
	}
	if (entity.hasComponent<CCircle>())
	{
		out << YAML::Key << "CircleShape";
		out << YAML::BeginMap;
		auto& cc = entity.getComponent<CCircle>();
		out << YAML::Key << "Radius" << YAML::Value << cc.radius;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CRectangle>())
	{
		out << YAML::Key << "RectangleShape";
		out << YAML::BeginMap;
		auto& rc = entity.getComponent<CRectangle>();
		out << YAML::Key << "Size" << YAML::Value << rc.size;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CSpriteRenderer>())
	{
		out << YAML::Key << "SpriteRenderer";
		out << YAML::BeginMap;
		
		auto& spriteRendererComponent = entity.getComponent<CSpriteRenderer>();
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
	if (entity.hasComponent<CCircleCollider>())
	{
		out << YAML::Key << "CircleCollider";
		out << YAML::BeginMap;
		auto& cc2d = entity.getComponent<CCircleCollider>();
		out << YAML::Key << "Radius" << YAML::Value << cc2d.radius;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CPhysicsMaterial>())
	{
		out << YAML::Key << "PhysicsMaterial";
		out << YAML::BeginMap;
		auto& pm = entity.getComponent<CPhysicsMaterial>();
		out << YAML::Key << "Mass" << YAML::Value << pm.mass;
		out << YAML::Key << "RestitutionCoefficient" << YAML::Value << pm.restitutionCoefficient;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CBoundingBox>())
	{
		out << YAML::Key << "AABB";
		out << YAML::BeginMap;

		auto& bc2d = entity.getComponent<CBoundingBox>();
		out << YAML::Key << "Offset" << YAML::Value << bc2d.offset;
		out << YAML::Key << "Size" << YAML::Value << bc2d.size;

		out << YAML::EndMap;
	}
	if (entity.hasComponent<CPolygonCollider>())
	{
		out << YAML::Key << "PolygonCollider";
		out << YAML::BeginMap;

		auto& pc2d = entity.getComponent<CPolygonCollider>();
		out << YAML::Key << "Offset" << YAML::Value << pc2d.offset;
		out << YAML::Key << "TexSize" << YAML::Value << pc2d.size;
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
	out << YAML::EndMap;
}

void SceneSerializer::Serialize(const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << m_Scene->m_Name;
	
	out << YAML::Key << "PhysicsConfig";
	out << YAML::BeginMap;
	out << YAML::Key << "Gravity" << YAML::Value << m_Scene->m_gravity;
	out << YAML::Key << "ExternalForce" << YAML::Value << m_Scene->m_externalForce;
	out << YAML::Key << "VelocityIterations" << YAML::Key << m_Scene->m_velocityIterations;
	out << YAML::Key << "PositionIterations" << YAML::Key << m_Scene->m_positionIterations;
	out << YAML::Key << "PhysicsColliders" << YAML::Key << m_Scene->m_drawPhysicsColliders;
	out << YAML::Key << "BroadphaseCollision" << YAML::Key << m_Scene->m_KDTreeBroadPhaseCollision;
	out << YAML::EndMap;
	
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	for (auto entity : m_Scene->m_entityManager.GetEntities())
	{
		if (entity.isActive())
		{
			SerializeEntity(out, entity);
		}
	}
	out << YAML::EndSeq;
	out << YAML::EndMap;

	std::ofstream fout(filepath);
	fout << out.c_str();
}

bool SceneSerializer::Deserialize(const std::filesystem::path& filepath)
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


	if (!data["Scene"])
	{
		std::cout << "no Scene data\n";
		return false;
	}

	m_Scene->m_Name = data["Scene"].as<std::string>();

	auto physicsConfig = data["PhysicsConfig"];
	if (physicsConfig)
	{
		m_Scene->m_gravity = physicsConfig["Gravity"].as<Vec2>();
		m_Scene->m_externalForce = physicsConfig["ExternalForce"].as<Vec2>();
		m_Scene->m_velocityIterations = physicsConfig["VelocityIterations"].as<int>();
		m_Scene->m_positionIterations = physicsConfig["PositionIterations"].as<int>();
		m_Scene->m_drawPhysicsColliders = physicsConfig["PhysicsColliders"].as<bool>();
		m_Scene->m_KDTreeBroadPhaseCollision = physicsConfig["BroadphaseCollision"].as<bool>();
	}

	auto entities = data["Entities"];
	if (entities)
	{
		for (auto entity : entities)
		{
			auto tag = entity["Entity"].as<std::string>();
			Entity deserializedEntity = m_Scene->m_entityManager.addEntity();
			deserializedEntity.addComponent<CTag>(tag);

			auto transformComponent = entity["Transform"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity.addComponent<CTransform>();
				tc.pos = transformComponent["Position"].as<Vec2>();
				tc.scale = transformComponent["Scale"].as<Vec2>();
				tc.angle = transformComponent["Angle"].as<float>();
				tc.velocity = transformComponent["Velocity"].as<Vec2>();
				tc.angularVelocity = transformComponent["AngularVelocity"].as<float>();
			}

			auto spriteRendererComponent = entity["SpriteRenderer"];
			if (spriteRendererComponent)
			{
				auto& src = deserializedEntity.addComponent<CSpriteRenderer>();
				if (spriteRendererComponent["TextureHandle"])
				{
					src.texture = spriteRendererComponent["TextureHandle"].as<AssetHandle>();
				}
				src.layer = spriteRendererComponent["Layer"].as<int>();
			}

			auto circleComponent = entity["CircleShape"];
			if (circleComponent)
			{
				auto& cc = deserializedEntity.addComponent<CCircle>();
				cc.radius = circleComponent["Radius"].as<float>();
			}

			auto rectangleComponent = entity["RectangleShape"];
			if (rectangleComponent)
			{
				auto& rc = deserializedEntity.addComponent<CRectangle>();
				rc.size = rectangleComponent["Size"].as<Vec2>();
			}
			/*auto animationComponent = entity["Animation"];
			if (animationComponent)
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
			auto circleColliderComponent = entity["CircleCollider"];
			if (circleColliderComponent)
			{
				auto& cc2d = deserializedEntity.addComponent<CCircleCollider>();
				cc2d.radius = circleColliderComponent["Radius"].as<float>();
			}
			auto physicsMaterialComponent = entity["PhysicsMaterial"];
			if (physicsMaterialComponent)
			{
				auto& pm = deserializedEntity.addComponent<CPhysicsMaterial>();
				pm.mass = physicsMaterialComponent["Mass"].as<float>();
				pm.restitutionCoefficient = physicsMaterialComponent["RestitutionCoefficient"].as<float>();
			}
			auto boundingBoxComponent = entity["AABB"];
			if (boundingBoxComponent)
			{
				auto& bc2d = deserializedEntity.addComponent<CBoundingBox>();
				bc2d.offset = boundingBoxComponent["Offset"].as<Vec2>();
				bc2d.size = boundingBoxComponent["Size"].as<Vec2>();
				bc2d.halfSize = bc2d.size / 2;
			}

			auto polygonColliderComponent = entity["PolygonCollider"];
			if (polygonColliderComponent)
			{
				auto& pc2d = deserializedEntity.addComponent<CPolygonCollider>();
				pc2d.offset = polygonColliderComponent["Offset"].as<Vec2>();
				pc2d.size = polygonColliderComponent["TexSize"].as<Vec2>();
				auto verticesData = polygonColliderComponent["Points"];
				auto elements = verticesData["Elements"];
				for (auto element : elements)
				{
					pc2d.colliderVertices.push_back(element.as<Vec2>());
				}
			}

			if (tag == "player")
			{
				m_Scene->m_player = deserializedEntity;
			}

		}
	}

	return true;
}
