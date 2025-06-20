#include "ScriptEngine.h"

#include "core/Logger.h"

#include <string>
#include <fstream> 

#include "mono/jit/jit.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/object.h"

struct ScriptEngineData
{
	MonoDomain* RootDomain = nullptr;
	MonoDomain* AppDomain = nullptr;

	MonoAssembly* CoreAssembly = nullptr;
};

static ScriptEngineData* s_Data = nullptr;

void ScriptEngine::Init()
{
	s_Data = new ScriptEngineData();

	InitMono();
}

void ScriptEngine::Shutdown()
{
	ShutdownMono();
	delete s_Data;
}


char* ReadBytes(const std::string& filepath, uint32_t* outSize)
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

MonoAssembly* LoadCSharpAssembly(const std::string& assemblyPath)
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

    MonoAssembly* assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
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

void ScriptEngine::InitMono()
{
	mono_set_assemblies_path("D:/Game Development/Game_Engine_Programming/Elysium/Elysium-Editor/mono/lib");

	MonoDomain* rootDomain = mono_jit_init("ElysiumJITRuntime");
	// assert rootdomain

	// store the root domain pointer
	s_Data->RootDomain = rootDomain;

	// Create an App domain
    s_Data->AppDomain = mono_domain_create_appdomain(const_cast<char*>("ElysiumScriptRuntime"), nullptr);

	s_Data->CoreAssembly = LoadCSharpAssembly("D:/Game Development/Game_Engine_Programming/Elysium/Elysium-Editor/Resources/Scripts/Elysium-ScriptCore.dll");
    PrintAssemblyTypes(s_Data->CoreAssembly);

    MonoImage* assemlyImage = mono_assembly_get_image(s_Data->CoreAssembly);
    MonoClass* monoClass = mono_class_from_name(assemlyImage, "Elysium", "Main");

    // 1) create an object (and call constructor)
    MonoObject* instance = mono_object_new(s_Data->AppDomain, monoClass);
    mono_runtime_object_init(instance);

    // 2) call function
    MonoMethod* printMessageFunc = mono_class_get_method_from_name(monoClass, "PrintMessage", 0);
    mono_runtime_invoke(printMessageFunc, instance, nullptr, nullptr);

    // 3) call function with param(s)
    MonoMethod* printIntFunc = mono_class_get_method_from_name(monoClass, "PrintInt", 1);
    int value = 5;
    void* param = &value;
    mono_runtime_invoke(printIntFunc, instance, &param, nullptr);
    MonoMethod* printIntsFunc = mono_class_get_method_from_name(monoClass, "PrintInts", 2);
    int value2 = 51;
    void* params[2] = {
        &value,
        &value2
    };
    mono_runtime_invoke(printIntsFunc, instance, params, nullptr);

    MonoString* monoString = mono_string_new(s_Data->AppDomain, "hello world from c++");
    MonoMethod* printCustomMessageFunc = mono_class_get_method_from_name(monoClass, "PrintCustomMessage", 1);
    void* stringParam = monoString;
    mono_runtime_invoke(printCustomMessageFunc, instance, &stringParam, nullptr);

}

void ScriptEngine::ShutdownMono()
{
    mono_domain_unload(s_Data->AppDomain);
    s_Data->AppDomain = nullptr;

    mono_jit_cleanup(s_Data->RootDomain);
    s_Data->RootDomain = nullptr;
}
