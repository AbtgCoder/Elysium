#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "core/UUID.h"
#include "core/Logger.h"
#include "core/KeyCodes.h"
#include "core/Input.h"

#include "Asset/AssetManager.h"

#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/Components.h"

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

#include <iostream>

static std::unordered_map<MonoType*, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityAddComponentFuncs;
static std::unordered_map<MonoType*, std::function<void(Entity)>> s_EntityRemoveComponentFuncs;

#define ESM_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Elysium.InternalCalls::" #Name, Name)

static void NativeLog(MonoString* string, int parameter)
{
	char* cStr = mono_string_to_utf8(string);
	std::string str(cStr);
	mono_free(cStr);
	std::cout << str << ", " << parameter << std::endl;
}

static void NativeLog_Vector(glm::vec3* parameter, glm::vec3* outResult)
{
	*outResult = glm::normalize(*parameter);
}

static float NativeLog_VectorDot(glm::vec3* parameter)
{
	return glm::dot(*parameter, *parameter);
}

static bool Entity_HasComponent(Elysium::UUID entityID, MonoReflectionType* componentType)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	MonoType* managedType = mono_reflection_type_get_type(componentType);

	if (s_EntityHasComponentFuncs.find(managedType) != s_EntityHasComponentFuncs.end())
	{
		return s_EntityHasComponentFuncs.at(managedType)(entity);
	}
	else
	{
		// throw error/warning: component type not registered
	}
}

static void Entity_AddComponent(Elysium::UUID entityID, MonoReflectionType* componentType)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	MonoType* managedType = mono_reflection_type_get_type(componentType);

	if (s_EntityAddComponentFuncs.find(managedType) != s_EntityAddComponentFuncs.end())
	{
		s_EntityAddComponentFuncs.at(managedType)(entity);
	}
	else
	{
		// throw error/warning: component type not registered
	}
}

static void Entity_RemoveComponent(Elysium::UUID entityID, MonoReflectionType* componentType)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	MonoType* managedType = mono_reflection_type_get_type(componentType);

	if (s_EntityRemoveComponentFuncs.find(managedType) != s_EntityRemoveComponentFuncs.end())
	{
		s_EntityRemoveComponentFuncs.at(managedType)(entity);
	}
	else
	{
		// throw error/warning: component type not registered
	}
}

static uint64_t Entity_FindEntityByName(MonoString* name)
{
	char* nameCStr = mono_string_to_utf8(name);

	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->FindEntityByName(nameCStr);
	mono_free(nameCStr);

	if (!entity)
	{
		return 0;
	}

	return entity.GetUUID(); // Return the UUID of the entity if found, or 0 if not found
}

static uint64_t Entity_CreateEntity(MonoString* name)
{
	char* nameCStr = mono_string_to_utf8(name);
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->AddEntity(nameCStr);
	mono_free(nameCStr);
	return entity.GetUUID(); // Return the UUID of the newly created entity
}

static MonoObject* GetScriptInstance(Elysium::UUID entityID)
{
	return ScriptEngine::GetManagedInstance(entityID);
}

static MonoString* TagComponent_GetTag(Elysium::UUID entityID)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// asse: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid
	std::string tag = entity.getComponent<CTag>().tag;

	return mono_string_new(ScriptEngine::GetAppDomain(), tag.c_str());
}

static void TagComponent_SetTag(Elysium::UUID entityID, MonoString* string)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// asse: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	char* tagCStr = mono_string_to_utf8(string);
	entity.getComponent<CTag>().tag = std::string(tagCStr);
	mono_free(tagCStr);
}

static void RectangleComponent_GetSize(Elysium::UUID entityID, glm::vec2* outSize)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	if (!entity.hasComponent<CRectangle>())
	{
		Logger::Log("Entity does not have a Rectangle component", "Script Engine", LOG_TYPE::WARNING);
		return;
	}

	*outSize = glm::vec2(entity.getComponent<CRectangle>().size.x, entity.getComponent<CRectangle>().size.y);
}

static void RectangleComponent_SetSize(Elysium::UUID entityID, glm::vec2* translation)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	if (!entity.hasComponent<CRectangle>())
	{
		Logger::Log("Entity does not have a Rectangle component", "Script Engine", LOG_TYPE::WARNING);
		return;
	}

	glm::vec2 t = *translation;
	entity.getComponent<CRectangle>().size = {t.x, t.y};
}

static void RectangleComponent_GetColor(Elysium::UUID entityID, glm::vec4* outColor)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	if (!entity.hasComponent<CRectangle>())
	{
		Logger::Log("Entity does not have a Rectangle component", "Script Engine", LOG_TYPE::WARNING);
		return;
	}

	*outColor = entity.getComponent<CRectangle>().color;
}

static void RectangleComponent_SetColor(Elysium::UUID entityID, glm::vec4* color)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	if (!entity.hasComponent<CRectangle>())
	{
		Logger::Log("Entity does not have a Rectangle component", "Script Engine", LOG_TYPE::WARNING);
		return;
	}

	entity.getComponent<CRectangle>().color = *color;
}

static void TransformComponent_GetTranslation(Elysium::UUID entityID, glm::vec3* outTranslation)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	*outTranslation = entity.getComponent<CTransform>().Translation;
}

static void TransformComponent_SetTranslation(Elysium::UUID entityID, glm::vec3* translation)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	entity.getComponent<CTransform>().Translation = *translation;
}

static void TransformComponent_GetRotation(Elysium::UUID entityID, glm::vec3* outRotation)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	*outRotation = entity.getComponent<CTransform>().Rotation;
}

static void TransformComponent_SetRotation(Elysium::UUID entityID, glm::vec3* rotation)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	entity.getComponent<CTransform>().Rotation = *rotation;
}

static void TransformComponent_GetScale(Elysium::UUID entityID, glm::vec3* outScale)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	*outScale = entity.getComponent<CTransform>().Scale;
}

static void TransformComponent_SetScale(Elysium::UUID entityID, glm::vec3* scale)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid

	entity.getComponent<CTransform>().Scale = *scale;
}

int Texture2D_GetWidth(uint64_t handle)
{
	auto texture = AssetManager::GetAsset<Texture2D>(handle);
	return texture ? texture->GetWidth() : 0;
}

int Texture2D_GetHeight(uint64_t handle)
{
	auto texture = AssetManager::GetAsset<Texture2D>(handle);
	return texture ? texture->GetHeight() : 0;
}

uint64_t SpriteRendererComponent_GetTexture(uint64_t entityID)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid
	if (!entity.hasComponent<CSpriteRenderer>())
	{
		Logger::Log("Entity does not have a SpriteRenderer component", "Script Engine", LOG_TYPE::WARNING);
		return 0;
	}
	return entity.getComponent<CSpriteRenderer>().texture;
}

void SpriteRendererComponent_SetTexture(uint64_t entityID, uint64_t textureID)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert: scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert: entity is valid
	if (!entity.hasComponent<CSpriteRenderer>())
	{
		Logger::Log("Entity does not have a SpriteRenderer component", "Script Engine", LOG_TYPE::WARNING);
		return;
	}
	entity.getComponent<CSpriteRenderer>().texture = textureID;
}

static void RigidBodyComponent_ApplyLinearImpulse(Elysium::UUID entityID, glm::vec2* impulse, glm::vec2* point)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert entity is valid

	auto& rb2d = entity.getComponent<CRigidBody>();
	PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
	body->ApplyImpulseToPoint(Vec2(impulse->x, impulse->y), Vec2(point->x, point->y));
}

static void RigidBodyComponent_ApplyLinearImpulseToCenter(Elysium::UUID entityID, glm::vec2* impulse)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert entity is valid

	auto& rb2d = entity.getComponent<CRigidBody>();
	PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
	body->ApplyImpulseToCenter(Vec2(impulse->x, impulse->y));
}

static void RigidBodyComponent_GetLinearVelocity(Elysium::UUID entityID, glm::vec2* outLinearVelocity)
{
	Scene* scene = ScriptEngine::GetSceneContext();
	// assert scene
	Entity entity = scene->GetEntityByUUID(entityID);
	// assert entity is valid

	auto& rb2d = entity.getComponent<CRigidBody>();
	PhysicsBody* body = (PhysicsBody*)rb2d.runtimeBody;
	const Vec2& linearVelocity = body->m_velocity;
	*outLinearVelocity = glm::vec2(linearVelocity.x, linearVelocity.y);
}

static bool Input_IsKeyDown(KeyCode key)
{
	return Input::IsKeyPressed(key);
}

template <typename TComponent>
static void RegisterComponent()
{
	std::string_view typeName = typeid(TComponent).name();
	size_t pos = typeName.find_last_of(" ");
	std::string_view componentName = typeName.substr(pos + 2);
	std::string managedTypeName = "Elysium." + std::string(componentName) + "Component";


	MonoType* managedType = mono_reflection_type_from_name(managedTypeName.data(), ScriptEngine::GetCoreAssemblyImage());
	if (!managedType)
	{
		Logger::Log("Failed to find managed type: " + managedTypeName, "Script Engine", LOG_TYPE::WARNING);
		return;
	}

	s_EntityHasComponentFuncs[managedType] = [](Entity entity) -> bool
	{
		return entity.hasComponent<TComponent>();
	};

	s_EntityAddComponentFuncs[managedType] = [](Entity entity) -> void
	{
		entity.addComponent<TComponent>();
	};

	s_EntityRemoveComponentFuncs[managedType] = [](Entity entity) -> void
	{
		entity.removeComponent<TComponent>();
	};

	Logger::Log("Succesfully Registered component: " + managedTypeName, "Script Engine");

}

void ScriptGlue::RegisterComponents()
{
	s_EntityHasComponentFuncs.clear();

	RegisterComponent<CTransform>();
	RegisterComponent<CTag>();
	RegisterComponent<CCamera>();
	RegisterComponent<CRectangle>();
	RegisterComponent<CSpriteRenderer>();
	RegisterComponent<CRigidBody>();
	//TODO: register other components as needed
}

void ScriptGlue::RegisterFunctions()
{
	ESM_ADD_INTERNAL_CALL(NativeLog);
	ESM_ADD_INTERNAL_CALL(NativeLog_Vector);
	ESM_ADD_INTERNAL_CALL(NativeLog_VectorDot);

	ESM_ADD_INTERNAL_CALL(GetScriptInstance);

	ESM_ADD_INTERNAL_CALL(Entity_CreateEntity);
	ESM_ADD_INTERNAL_CALL(Entity_HasComponent);
	ESM_ADD_INTERNAL_CALL(Entity_AddComponent);
	ESM_ADD_INTERNAL_CALL(Entity_RemoveComponent);
	ESM_ADD_INTERNAL_CALL(Entity_FindEntityByName);

	ESM_ADD_INTERNAL_CALL(TagComponent_GetTag);
	ESM_ADD_INTERNAL_CALL(TagComponent_SetTag);

	ESM_ADD_INTERNAL_CALL(RectangleComponent_GetSize);
	ESM_ADD_INTERNAL_CALL(RectangleComponent_SetSize);
	ESM_ADD_INTERNAL_CALL(RectangleComponent_GetColor);
	ESM_ADD_INTERNAL_CALL(RectangleComponent_SetColor);

	ESM_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
	ESM_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);
	ESM_ADD_INTERNAL_CALL(TransformComponent_GetRotation);
	ESM_ADD_INTERNAL_CALL(TransformComponent_SetRotation);
	ESM_ADD_INTERNAL_CALL(TransformComponent_GetScale);
	ESM_ADD_INTERNAL_CALL(TransformComponent_SetScale);

	ESM_ADD_INTERNAL_CALL(Texture2D_GetWidth);
	ESM_ADD_INTERNAL_CALL(Texture2D_GetHeight);
	ESM_ADD_INTERNAL_CALL(SpriteRendererComponent_GetTexture);
	ESM_ADD_INTERNAL_CALL(SpriteRendererComponent_SetTexture);

	ESM_ADD_INTERNAL_CALL(RigidBodyComponent_ApplyLinearImpulse);
	ESM_ADD_INTERNAL_CALL(RigidBodyComponent_ApplyLinearImpulseToCenter);
	ESM_ADD_INTERNAL_CALL(RigidBodyComponent_GetLinearVelocity);

	ESM_ADD_INTERNAL_CALL(Input_IsKeyDown);
}
