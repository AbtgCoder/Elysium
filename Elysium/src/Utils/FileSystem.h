#pragma once

#include <filesystem>

namespace Elysium
{
	class FileSystem
	{
	public:
		static void Init();

		static const std::filesystem::path& GetEngineRootDir();
		static const std::filesystem::path& GetEditorRootDir();
		static std::filesystem::path GetResourcePath(const std::filesystem::path& relative);
	private:
		static std::filesystem::path s_EngineRootDir;
		static std::filesystem::path s_EditorRootDir;
	};
}