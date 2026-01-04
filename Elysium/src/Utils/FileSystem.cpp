#include "FileSystem.h"

#ifdef _WIN32
	#include <Windows.h>
#endif

#include "core/Logger.h"

namespace Elysium
{
	std::filesystem::path FileSystem::s_EngineRootDir;
	std::filesystem::path FileSystem::s_EditorRootDir;

	void FileSystem::Init()
	{
		std::filesystem::path executableDir;

#ifdef _WIN32
		wchar_t buffer[MAX_PATH];
		GetModuleFileNameW(nullptr, buffer, MAX_PATH);
		executableDir = std::filesystem::path(buffer).parent_path();
#else
		executableDir = std::filesystem::current_path();
#endif

		s_EngineRootDir = executableDir.parent_path().parent_path().parent_path();

		s_EditorRootDir = s_EngineRootDir / "Elysium-Editor";

		Logger::Log(s_EngineRootDir.string());
		Logger::Log(s_EditorRootDir.string());
	}

	const std::filesystem::path& FileSystem::GetEngineRootDir()
	{
		return s_EngineRootDir;
	}

	const std::filesystem::path& FileSystem::GetEditorRootDir()
	{
		return s_EditorRootDir;
	}

	std::filesystem::path FileSystem::GetResourcePath(const std::filesystem::path& relative)
	{
		return s_EngineRootDir / relative;
	}
}