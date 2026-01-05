#pragma once

#include <filesystem>

namespace Platform
{
	bool OpenFileInShell(const std::filesystem::path& path, const std::filesystem::path& workingDir = {});
}
