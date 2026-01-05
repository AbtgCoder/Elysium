#pragma once

#include <string>
#include <filesystem>

namespace Utils::Process
{
	// runs a process using CreateProcessW, cmdline should be the full cmd string
	// working directory is optional
	// outExitCode returns the process exit code if provided
	bool Run(const std::wstring& commandLine, const std::filesystem::path& workingDirectory = {}, int* outExitCode = nullptr);
	
	struct Result
	{
		bool Sucess = false;
		int ExitCode = 1;
		std::string Output; // combined stdout + stderr
	};

	Result RunAndCapture(const std::wstring& cmdLine, const std::filesystem::path& workingDirectory);
}