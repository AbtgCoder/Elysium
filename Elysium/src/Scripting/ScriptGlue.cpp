#include "ScriptGlue.h"

#include "ScriptEngine.h"

#include "core/UUID.h"
#include "core/Logger.h"
#include "core/KeyCodes.h"
#include "core/Input.h"

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


static MonoObject* GetScriptInstance(Elysium::UUID entityID)
{
	return ScriptEngine::GetManagedInstance(entityID);
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
	else
	{
		Logger::Log("Succesfully Registered component: " + managedTypeName, "Script Engine");
	}

	s_EntityHasComponentFuncs[managedType] = [](Entity entity) -> bool
	{
		return entity.hasComponent<TComponent>();
	};
}

void ScriptGlue::RegisterComponents()
{
	s_EntityHasComponentFuncs.clear();

	RegisterComponent<CTransform>();
	RegisterComponent<CTag>();
	RegisterComponent<CCamera>();
	RegisterComponent<CRectangle>();
	//TODO: register other components as needed
}

void ScriptGlue::RegisterFunctions()
{
	ESM_ADD_INTERNAL_CALL(NativeLog);
	ESM_ADD_INTERNAL_CALL(NativeLog_Vector);
	ESM_ADD_INTERNAL_CALL(NativeLog_VectorDot);

	ESM_ADD_INTERNAL_CALL(GetScriptInstance);

	ESM_ADD_INTERNAL_CALL(Entity_HasComponent);
	ESM_ADD_INTERNAL_CALL(Entity_FindEntityByName);

	ESM_ADD_INTERNAL_CALL(TransformComponent_GetTranslation);
	ESM_ADD_INTERNAL_CALL(TransformComponent_SetTranslation);

	ESM_ADD_INTERNAL_CALL(Input_IsKeyDown);
}
