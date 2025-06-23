#include "ScriptEngine.h"

#include "ScriptGlue.h"

#include "core/Logger.h"

#include "core/uuid.h"

#include <string>
#include <fstream> 

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

namespace Utils
{
    //TODO: move to filesystem class
    static char* ReadBytes(const std::filesystem::path& filepath, uint32_t* outSize)
    {
        // Open the file stream
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);
        if (!stream.is_open())
        {
            // Handle error if file cannot be opened
            return nullptr;
        }

        // Get the file size
        std::streamsize size = stream.tellg();
        if (size <= 0)
        {
            // Handle empty or invalid file
            return nullptr;
        }

        // Allocate memory for the file content
        char* buffer = new char[size];

        // Read the file content
        stream.seekg(0, std::ios::beg);
        if (!stream.read(buffer, size))
        {
            // Handle read error
            delete[] buffer;
            return nullptr;
        }

        *outSize = static_cast<uint32_t>(size);

        return buffer;
    }

    static MonoAssembly* LoadMonoAssembly(const std::filesystem::path& assemblyPath)
    {
        uint32_t fileSize = 0;
        char* fileData = ReadBytes(assemblyPath, &fileSize);

        //NOTE: we cant use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage* image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK)
        {
            const char* errorMessage = mono_image_strerror(status);
            // log some error message using the errorMessage data
            return nullptr;
        }

        MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.string().c_str(), &status, 0);
        mono_image_close(image);

        // delete the file data
        delete[] fileData;

        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly* assembly)
    {
        MonoImage* image = mono_assembly_get_image(assembly);
        const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++)
        {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            printf("%s.%s\n", nameSpace, name);
        }
    }

}

struct ScriptEngineData
{
	MonoDomain* RootDomain = nullptr;
	MonoDomain* AppDomain = nullptr;

	MonoAssembly* CoreAssembly = nullptr;
    MonoImage* CoreAssemblyImage = nullptr;

    ScriptClass EntityClass;

	std::unordered_map<std::string, std::shared_ptr<ScriptClass>> EntityClasses;
    std::unordered_map<Elysium::UUID, std::shared_ptr<ScriptInstance>> EntityInstances;

    // runtime
	Scene* SceneContext = nullptr;
};

static ScriptEngineData* s_Data = nullptr;

void ScriptEngine::Init()
{
	s_Data = new ScriptEngineData();

	InitMono();
    LoadAssembly("D:/Game Development/Game_Engine_Programming/Elysium/Elysium-Editor/Resources/Scripts/Elysium-ScriptCore.dll");

	LoadAssemblyClasses(s_Data->CoreAssembly);


    ScriptGlue::RegisterComponents();
    ScriptGlue::RegisterFunctions();

    s_Data->EntityClass = ScriptClass("Elysium", "Entity");

#if 0
    // 1) create an object (and call constructor)
    MonoObject* instance = s_Data->EntityClass.Instantiate();

    // 2) call function
    MonoMethod* printMessageFunc = s_Data->EntityClass.GetMethod("PrintMessage", 0);
    s_Data->EntityClass.InvokeMethod(instance, printMessageFunc);

    // 3) call function with param(s)
    MonoMethod* printIntFunc = s_Data->EntityClass.GetMethod("PrintInt", 1);
    int value = 5;
    void* param = &value;
    s_Data->EntityClass.InvokeMethod(instance, printIntFunc, &param);
    MonoMethod* printIntsFunc = s_Data->EntityClass.GetMethod("PrintInts", 2);
    int value2 = 51;
    void* params[2] = {
        &value,
        &value2
    };
    s_Data->EntityClass.InvokeMethod(instance, printIntsFunc, params);

    MonoString* monoString = mono_string_new(s_Data->AppDomain, "hello world from c++");
    MonoMethod* printCustomMessageFunc = s_Data->EntityClass.GetMethod("PrintCustomMessage", 1);
    void* stringParam = monoString;
    s_Data->EntityClass.InvokeMethod(instance, printCustomMessageFunc, &stringParam);
#endif
}

void ScriptEngine::Shutdown()
{
	ShutdownMono();
	delete s_Data;
}



Scene* ScriptEngine::GetSceneContext()
{
    return s_Data->SceneContext;
}

std::unordered_map<std::string, std::shared_ptr<ScriptClass>>& ScriptEngine::GetEntityClasses()
{
	return s_Data->EntityClasses;
}

MonoImage* ScriptEngine::GetCoreAssemblyImage()
{
    return s_Data->CoreAssemblyImage;
}

void ScriptEngine::InitMono()
{
    mono_set_assemblies_path("D:/Game Development/Game_Engine_Programming/Elysium/Elysium-Editor/mono/lib");

    MonoDomain* rootDomain = mono_jit_init("ElysiumJITRuntime");
    // assert rootdomain

    // store the root domain pointer
    s_Data->RootDomain = rootDomain;
}
	

void ScriptEngine::ShutdownMono()
{
   // mono_domain_unload(s_Data->AppDomain);
    s_Data->AppDomain = nullptr;

   // mono_jit_cleanup(s_Data->RootDomain);
    s_Data->RootDomain = nullptr;
}

MonoObject* ScriptEngine::InstantiateClass(MonoClass* monoClass)
{
    MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
    mono_runtime_object_init(instance);
    return instance;
}

void ScriptEngine::LoadAssemblyClasses(MonoAssembly* assembly)
{
    s_Data->EntityClasses.clear();

	MonoImage* image = mono_assembly_get_image(assembly);
    const MonoTableInfo* typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
    int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

	MonoClass* entityClass = mono_class_from_name(image, "Elysium", "Entity");

    for (int32_t i = 0; i < numTypes; i++)
    {
        uint32_t cols[MONO_TYPEDEF_SIZE];
        mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);
        const char* nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
        const char* name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);
        std::string fullClassName; 
        if (strlen(nameSpace) == 0)
            fullClassName = std::string(name);
        else
			fullClassName = std::string(nameSpace) + "." + std::string(name);

		MonoClass* monoClass = mono_class_from_name(image, nameSpace, name);

        if (monoClass == entityClass)
        {
            // skip the entity class
            continue;
		}

		bool isEntityClass = mono_class_is_subclass_of(monoClass, entityClass, false);
        if (isEntityClass)
            s_Data->EntityClasses[fullClassName] = std::make_shared<ScriptClass>(nameSpace, name);
	}   
}

void ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
{
    // create an App domain
    s_Data->AppDomain = mono_domain_create_appdomain(const_cast<char*>("ElysiumScriptRuntime"), nullptr);
    mono_domain_set(s_Data->AppDomain, true);

    s_Data->CoreAssembly = Utils::LoadMonoAssembly(filepath);
    s_Data->CoreAssemblyImage = mono_assembly_get_image(s_Data->CoreAssembly);
}

bool ScriptEngine::EntityClassExists(const std::string& fullClassName)
{
    return s_Data->EntityClasses.find(fullClassName) != s_Data->EntityClasses.end();
}

void ScriptEngine::OnRuntimeStart(Scene* scene)
{
	s_Data->SceneContext = scene;
}

void ScriptEngine::OnCreateEntity(Entity entity)
{
    const auto& sc = entity.getComponent<CScript>();
    if (ScriptEngine::EntityClassExists(sc.ClassName))
    {
        auto scriptInstance = std::make_shared<ScriptInstance>(s_Data->EntityClasses[sc.ClassName], entity);
        s_Data->EntityInstances[entity.GetUUID()] = scriptInstance;
        // invoke OnCreate method
        scriptInstance->InvokeOnCreate();
    }
    else
    {
		// error: script class does not exist
	}
}

void ScriptEngine::OnUpdateEntity(Entity entity, float deltaTime)
{
	Elysium::UUID entityID = entity.GetUUID();

	// assert s_Data->EntityInstances.find(entityID) != s_Data->EntityInstances.end();

	std::shared_ptr<ScriptInstance> scriptInstance = s_Data->EntityInstances[entityID];
	scriptInstance->InvokeOnUpdate(deltaTime);
}

void ScriptEngine::OnRuntimeStop()
{
    s_Data->SceneContext = nullptr;
    // clear all entity instances
    s_Data->EntityInstances.clear();
}


ScriptClass::ScriptClass(const std::string& classNamespace, const std::string& className)
    : m_ClassNamespace(classNamespace), m_ClassName(className)
{
    m_MonoClass = mono_class_from_name(s_Data->CoreAssemblyImage, classNamespace.c_str(), className.c_str());
}

MonoObject* ScriptClass::Instantiate()
{
    return ScriptEngine::InstantiateClass(m_MonoClass);
}

MonoMethod* ScriptClass::GetMethod(const std::string& name, int parameterCount)
{
    return mono_class_get_method_from_name(m_MonoClass, name.c_str(), parameterCount);
}

MonoObject* ScriptClass::InvokeMethod(MonoObject* instance, MonoMethod* method, void** params)
{
    return mono_runtime_invoke(method, instance, params, nullptr);
}

ScriptInstance::ScriptInstance(std::shared_ptr<ScriptClass> scriptClass, Entity entity)
    : m_ScriptClass(scriptClass)
{
    m_Instance = scriptClass->Instantiate();

	m_Constructor = s_Data->EntityClass.GetMethod(".ctor", 1); // retrieve the constructor method from the parent Entity class
	m_OnCreateMethod = scriptClass->GetMethod("OnCreate", 0);
	m_OnUpdateMethod = scriptClass->GetMethod("OnUpdate", 1);

	// call entity constructor with the instance of the child script class
    {
		Elysium::UUID entityID = entity.GetUUID();
        void* params[1] = { &entityID };
		s_Data->EntityClass.InvokeMethod(m_Instance, m_Constructor, params);
    }
}

void ScriptInstance::InvokeOnCreate()
{
    if (m_OnCreateMethod)
    {
		m_ScriptClass->InvokeMethod(m_Instance, m_OnCreateMethod, nullptr);
    }
}

void ScriptInstance::InvokeOnUpdate(float deltaTime)
{
    if  (m_OnUpdateMethod)
    {
        void* params[1] = { &deltaTime };
        m_ScriptClass->InvokeMethod(m_Instance, m_OnUpdateMethod, params);
	}
}
