#include "SceneSerializer.h"

#include "Core/UUID.h"
#include "Project/Project.h"

#include "Scripting/ScriptEngine.h"

#include "Asset/AnimationImporter.h"
#include "Asset/AssetManager.h"

#include <fstream>

static std::string RigidBody2DBodyTypeToString(CRigidBody::BodyType bodyType)
{
	switch (bodyType)
	{
	case CRigidBody::BodyType::Static:    return "Static";
	case CRigidBody::BodyType::Dynamic:   return "Dynamic";
	case CRigidBody::BodyType::Kinematic: return "Kinematic";
	}

	// assert invalid value type
	return {};
}

static CRigidBody::BodyType RigidBody2DBodyTypeFromString(const std::string& bodyTypeString)
{
	if (bodyTypeString == "Static")    return CRigidBody::BodyType::Static;
	if (bodyTypeString == "Dynamic")   return CRigidBody::BodyType::Dynamic;
	if (bodyTypeString == "Kinematic") return CRigidBody::BodyType::Kinematic;

	// assert invalid value type

	return CRigidBody::BodyType::Static;
}

#define WRITE_SCRIPT_FIELD(FieldType, Type)           \
			case ScriptFieldType::FieldType:          \
				out << scriptField.GetValue<Type>();  \
				break

#define READ_SCRIPT_FIELD(FieldType, Type)             \
	case ScriptFieldType::FieldType:                   \
	{                                                  \
		Type data = scriptField["Data"].as<Type>();    \
		fieldInstance.SetValue(data);                  \
		break;                                         \
	}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene>& Scene)
	: m_Scene(Scene)
{
}

void SceneSerializer::SerializeEntity(YAML::Emitter& out, Entity entity)
{
	out << YAML::BeginMap; // Entity
	out << YAML::Key << "Entity" << YAML::Value << entity.getComponent<CId>().id;
	if (entity.hasComponent<CTag>())
	{
		out << YAML::Key << "Tag";
		out << YAML::BeginMap;
		auto& tag = entity.getComponent<CTag>().tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CTransform>())
	{
		out << YAML::Key << "Transform";
		out << YAML::BeginMap; // Transform Component

		auto& tc = entity.getComponent<CTransform>();
		out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
		out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
		out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

		out << YAML::Key << "GlobalTranslation" << YAML::Value << tc.GlobalTranslation;
		out << YAML::Key << "GlobalRotation" << YAML::Value << tc.GlobalRotation;
		out << YAML::Key << "GlobalScale" << YAML::Value << tc.GlobalScale;
		//out << YAML::Key << "Velocity" << YAML::Value << tc.velocity;
		//out << YAML::Key << "AngularVelocity" << YAML::Value << tc.angularVelocity;

		out << YAML::EndMap; // TransformComponent
	}
	if (entity.hasComponent<CParent>())
	{
		out << YAML::Key << "ParentComponent";
		out << YAML::BeginMap;
		auto& pc = entity.getComponent<CParent>();
		out << YAML::Key << "HasParent" << YAML::Value << pc.HasParent;
		Elysium::UUID pId = 0;
		if (pc.HasParent)
		{
			pId = pc.ParentID;
		}
		out << YAML::Key << "ParentID" << YAML::Value << pId;
		out << YAML::Key << "Children" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		for (auto& cId : pc.Children)
		{
			out << cId;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CCamera>())
	{
		out << YAML::Key << "Camera";
		out << YAML::BeginMap;
		auto& camera = entity.getComponent<CCamera>();

		out << YAML::Key << "Camera" << YAML::Value;
		out << YAML::BeginMap; // scene camera
		out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.Camera.GetProjectionType();
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.Camera.GetPerspectiveVerticalFOV();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.Camera.GetPerspectiveNearClip();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.Camera.GetPerspectiveFarClip();
		out << YAML::Key << "OrthographicSize" << YAML::Value << camera.Camera.GetOrthographicSize();
		out << YAML::Key << "OrthographicNear" << YAML::Value << camera.Camera.GetOrthographicNearClip();
		out << YAML::Key << "OrthographicFar" << YAML::Value << camera.Camera.GetOrthographicFarClip();
		out << YAML::EndMap;

		out << YAML::Key << "Primary" << YAML::Value << camera.primary;
		out << YAML::Key << "BackgroundColor" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		out << (float)camera.backgroundColor.x;
		out << (float)camera.backgroundColor.y;
		out << (float)camera.backgroundColor.z;
		out << (float)camera.backgroundColor.w;
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CScript>())
	{
		out << YAML::Key << "ScriptComponent";
		out << YAML::BeginMap;
		auto& scriptComponent = entity.getComponent<CScript>();
		out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.ClassName;

		// Fields
		std::shared_ptr<ScriptClass> entityClass = ScriptEngine::GetEntityClass(scriptComponent.ClassName);
		const auto& fields = entityClass->GetFields();
		if (fields.size() > 0)
		{
			out << YAML::Key << "ScriptFields" << YAML::Value;
			auto& entityFields = ScriptEngine::GetScriptFieldMap(entity);
			out << YAML::BeginSeq;
			for (const auto& [name, field] : fields)
			{
				if (entityFields.find(name) == entityFields.end())
					continue;

				out << YAML::BeginMap; // ScriptField
				
				out << YAML::Key << "Name" << YAML::Value << name;
				out << YAML::Key << "Type" << YAML::Value << Utils::ScriptFieldTypeToString(field.Type);

				out << YAML::Key << "Data" << YAML::Value;
				ScriptFieldInstance& scriptField = entityFields.at(name);
				switch (field.Type)
				{
					WRITE_SCRIPT_FIELD(Float, float);
					WRITE_SCRIPT_FIELD(Double, double);
					WRITE_SCRIPT_FIELD(Bool, bool);
					WRITE_SCRIPT_FIELD(Char, char);
					WRITE_SCRIPT_FIELD(Byte, int8_t);
					WRITE_SCRIPT_FIELD(Short, int16_t);
					WRITE_SCRIPT_FIELD(Int, int32_t);
					WRITE_SCRIPT_FIELD(Long, int64_t);
					WRITE_SCRIPT_FIELD(UByte, uint8_t);
					WRITE_SCRIPT_FIELD(UShort, uint16_t);
					WRITE_SCRIPT_FIELD(UInt, uint32_t);
					WRITE_SCRIPT_FIELD(ULong, uint64_t);
					//WRITE_SCRIPT_FIELD(Vector2, glm::vec2);
					WRITE_SCRIPT_FIELD(Vector3, glm::vec3);
					WRITE_SCRIPT_FIELD(Texture2D, uint64_t);
					WRITE_SCRIPT_FIELD(Entity, Elysium::UUID);
				}

				out << YAML::EndMap; // ScriptField
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap;
	}
	if (entity.hasComponent<CCircle>())
	{
		out << YAML::Key << "CircleShape";
		out << YAML::BeginMap;
		auto& cc = entity.getComponent<CCircle>();
		out << YAML::Key << "Radius" << YAML::Value << cc.radius;
		out << YAML::Key << "Color" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		out << (float)cc.color.x;
		out << (float)cc.color.y;
		out << (float)cc.color.z;
		out << (float)cc.color.w;
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CRectangle>())
	{
		out << YAML::Key << "RectangleShape";
		out << YAML::BeginMap;
		auto& rc = entity.getComponent<CRectangle>();
		out << YAML::Key << "Size" << YAML::Value << rc.size;
		out << YAML::Key << "Color" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		out << (float)rc.color.x;
		out << (float)rc.color.y;
		out << (float)rc.color.z;
		out << (float)rc.color.w;
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}
	if (entity.hasComponent<CPolygon>())
	{
		out << YAML::Key << "PolygonShape";
		out << YAML::BeginMap;
		auto& pc = entity.getComponent<CPolygon>();
		out << YAML::Key << "Sides" << YAML::Value << pc.sides;
		out << YAML::Key << "Size" << YAML::Value << pc.size;
		out << YAML::Key << "Color" << YAML::Value << YAML::Flow;
		out << YAML::BeginSeq;
		out << (float)pc.color.x;
		out << (float)pc.color.y;
		out << (float)pc.color.z;
		out << (float)pc.color.w;
		out << YAML::EndSeq;
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

	if (entity.hasComponent<CAnimator>())
	{
		out << YAML::Key << "Animator";
		out << YAML::BeginMap;

		auto& animController = entity.getComponent<CAnimator>().Controller;

		out << YAML::Key << "CurrentState" << YAML::Value << animController.m_CurrentState;

		out << YAML::Key << "AnimationStates" << YAML::BeginSeq;
		for (auto [stateName, animState] : animController.m_States)
		{
			out << YAML::BeginMap;

			out << YAML::Key << "Name" << YAML::Value << stateName;
			// when saving the scene, save the animation clip as well at its proper location..TODO: maybe find a better way to handle this
			AnimationImporter::SaveAnimationClip(animState.Clip, Project::GetActive()->GetEditorAssetManager()->GetFilePath(animState.Clip->Handle));
			out << YAML::Key << "AnimationClipHandle" << YAML::Value << animState.Clip->Handle; //NOTE: this should always be a valid asset handle, as whenever we add the clip, we always import it as an asset as well...

			out << YAML::EndMap;
		}
		out << YAML::EndSeq;

		out << YAML::EndMap;
	}

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
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << pm.restitutionThreshold;
		out << YAML::Key << "Friction" << YAML::Value << pm.friction;
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
	if (entity.hasComponent<CRigidBody>())
	{
		out << YAML::Key << "Rigidbody2D";
		out << YAML::BeginMap;
		auto& rb2d = entity.getComponent<CRigidBody>();
		out << YAML::Key << "BodyType" << YAML::Value << RigidBody2DBodyTypeToString(rb2d.Type);
		out << YAML::EndMap; 
	}
	if (entity.hasComponent<CJoint>())
	{
		out << YAML::Key << "Joint";
		out << YAML::BeginMap;
		auto& jc = entity.getComponent<CJoint>();
		out << YAML::Key << "Entity1ID" << YAML::Value << jc.entity1Id;
		out << YAML::Key << "Entity2ID" << YAML::Value << jc.entity2Id;
		out << YAML::Key << "AnchorPosition" << YAML::Value << jc.anchorPos;
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
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			std::string name;
			auto tagComponent = entity["Tag"];
			if (tagComponent)
				name = tagComponent["Tag"].as<std::string>();
			Entity deserializedEntity = m_Scene->AddEntityWithUUID(uuid, name);

			auto transformComponent = entity["Transform"];
			if (transformComponent)
			{
				auto& tc = deserializedEntity.getComponent<CTransform>();
				tc.Translation = transformComponent["Translation"].as<glm::vec3>();
				tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
				tc.Scale = transformComponent["Scale"].as<glm::vec3>();

				tc.GlobalTranslation = transformComponent["GlobalTranslation"].as<glm::vec3>();
				tc.GlobalRotation = transformComponent["GlobalRotation"].as<glm::vec3>();
				tc.GlobalScale = transformComponent["GlobalScale"].as<glm::vec3>();
			}


			auto parentComponent = entity["ParentComponent"];
			if (parentComponent)
			{
				auto& pc = deserializedEntity.addComponent<CParent>();
				pc.HasParent = parentComponent["HasParent"].as<bool>();
				if (pc.HasParent)
					pc.ParentID = parentComponent["ParentID"].as<uint64_t>();
				auto children = parentComponent["Children"];
				for (auto childId : children)
				{
					pc.Children.push_back(childId.as<uint64_t>());
				}
			}

			auto cameraComponent = entity["Camera"];
			if (cameraComponent)
			{
				auto& camera = deserializedEntity.addComponent<CCamera>();

				auto cameraProps = cameraComponent["Camera"];
				camera.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());

				camera.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
				camera.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
				camera.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());

				camera.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
				camera.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
				camera.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());

				camera.primary = cameraComponent["Primary"].as<bool>();
				auto colorArray = cameraComponent["BackgroundColor"];
				if (colorArray)
				{
					camera.backgroundColor = glm::vec4(
						colorArray[0].as<float>(),
						colorArray[1].as<float>(),
						colorArray[2].as<float>(),
						colorArray[3].as<float>()
					);
				}
			}

			auto scriptComponent = entity["ScriptComponent"];
			if (scriptComponent)
			{
				auto& script = deserializedEntity.addComponent<CScript>();
				script.ClassName = scriptComponent["ClassName"].as<std::string>();

				auto scriptFields = scriptComponent["ScriptFields"];
				if (scriptFields)
				{
					std::shared_ptr<ScriptClass> entityClass = ScriptEngine::GetEntityClass(script.ClassName);
					// assert entity class
					const auto& fields = entityClass->GetFields();
					auto& entityFields = ScriptEngine::GetScriptFieldMap(deserializedEntity);

					for (auto scriptField : scriptFields)
					{
						std::string name = scriptField["Name"].as<std::string>();
						std::string typeString = scriptField["Type"].as<std::string>();
						ScriptFieldType type = Utils::ScriptFieldTypeFromString(typeString);

						ScriptFieldInstance& fieldInstance = entityFields[name];

						if (fields.find(name) == fields.end())
							continue;

						fieldInstance.Field = fields.at(name);

						switch (type)
						{
							READ_SCRIPT_FIELD(Float, float);
							READ_SCRIPT_FIELD(Double, double);
							READ_SCRIPT_FIELD(Bool, bool);
							READ_SCRIPT_FIELD(Char, char);
							READ_SCRIPT_FIELD(Byte, int8_t);
							READ_SCRIPT_FIELD(Short, int16_t);
							READ_SCRIPT_FIELD(Int, int32_t);
							READ_SCRIPT_FIELD(Long, int64_t);
							READ_SCRIPT_FIELD(UByte, uint8_t);
							READ_SCRIPT_FIELD(UShort, uint16_t);
							READ_SCRIPT_FIELD(UInt, uint32_t);
							READ_SCRIPT_FIELD(ULong, uint64_t);
							READ_SCRIPT_FIELD(Vector3, glm::vec3);
							READ_SCRIPT_FIELD(Texture2D, uint64_t);
							READ_SCRIPT_FIELD(Entity, Elysium::UUID);
						}
					}
				}
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

			auto animatorComponent = entity["Animator"];
			if (animatorComponent)
			{
				auto& animComponent = deserializedEntity.addComponent<CAnimator>();

				AnimationController animController;
				animController.m_CurrentState = animatorComponent["CurrentState"].as<std::string>();

				auto animStates = animatorComponent["AnimationStates"];
				if (animStates)
				{
					for (auto animState : animStates)
					{
						animController.AddState(
							animState["Name"].as<std::string>(),
							AssetManager::GetAsset<AnimationClip>(animState["AnimationClipHandle"].as<AssetHandle>())
						);
					}
				}

				animComponent.Controller = animController;
			}

			auto circleComponent = entity["CircleShape"];
			if (circleComponent)
			{
				auto& cc = deserializedEntity.addComponent<CCircle>();
				cc.radius = circleComponent["Radius"].as<float>();
				auto colorArray = circleComponent["Color"];
				if (colorArray)
				{
					cc.color = glm::vec4(
						colorArray[0].as<float>(),
						colorArray[1].as<float>(),
						colorArray[2].as<float>(),
						colorArray[3].as<float>()
					);
				}
			}

			auto rectangleComponent = entity["RectangleShape"];
			if (rectangleComponent)
			{
				auto& rc = deserializedEntity.addComponent<CRectangle>();
				rc.size = rectangleComponent["Size"].as<Vec2>();
				auto colorArray = rectangleComponent["Color"];
				if (colorArray)
				{
					rc.color = glm::vec4(
						colorArray[0].as<float>(),
						colorArray[1].as<float>(),
						colorArray[2].as<float>(),
						colorArray[3].as<float>()
					);
				}
			}

			auto polygonShapeComponent = entity["PolygonShape"];
			if (polygonShapeComponent)
			{
				auto& pc = deserializedEntity.addComponent<CPolygon>();
				pc.sides = polygonShapeComponent["Sides"].as<int>(); // TODO: clamp to given range, or just assert ??
				pc.size = polygonShapeComponent["Size"].as<float>();
				auto colorArray = polygonShapeComponent["Color"];
				if (colorArray)
				{
					pc.color = glm::vec4(
						colorArray[0].as<float>(),
						colorArray[1].as<float>(),
						colorArray[2].as<float>(),
						colorArray[3].as<float>()
					);
				}
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
			
			auto rigidbodyComponent = entity["Rigidbody2D"];
			if (rigidbodyComponent)
			{
				auto& rb2d = deserializedEntity.addComponent<CRigidBody>();
				rb2d.Type = RigidBody2DBodyTypeFromString(rigidbodyComponent["BodyType"].as<std::string>());
			}
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
				auto restitutionThreshold = physicsMaterialComponent["RestitutionThreshold"];
				if (restitutionThreshold)
					pm.restitutionThreshold = restitutionThreshold.as<float>();
				pm.friction = physicsMaterialComponent["Friction"].as<float>();
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

			auto jointComponent = entity["Joint"];
			if (jointComponent)
			{
				auto& jc = deserializedEntity.addComponent<CJoint>();
				jc.entity1Id = jointComponent["Entity1ID"].as<Elysium::UUID>(); // TODO: this should be equal to entity's uuid else there might be problems ??
				jc.entity2Id = jointComponent["Entity2ID"].as<Elysium::UUID>(); 
				jc.anchorPos = jointComponent["AnchorPosition"].as<Vec2>();
			}

			//if (tag == "player")
			//{
			//	m_Scene->m_player = deserializedEntity;
			//}

		}
	}

	return true;
}
