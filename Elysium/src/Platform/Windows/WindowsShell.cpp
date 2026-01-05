#include "WindowsShell.h"

#include <Windows.h>
#include <shellapi.h>

namespace Platform
{
	bool OpenFileInShell(const std::filesystem::path& path, const std::filesystem::path& workingDir)
	{
		if (!std::filesystem::exists(path))
			return false;

		HINSTANCE result = ShellExecuteW(
			nullptr,
			L"open",
			path.wstring().c_str(),
			nullptr,
			workingDir.empty() ? nullptr : workingDir.wstring().c_str(),
			SW_SHOWNORMAL
		);

		return reinterpret_cast<intptr_t>(result) > 32;
	}
}