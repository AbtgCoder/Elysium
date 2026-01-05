#include "Utils/Process.h"
#include "core/Logger.h"

#include "Windows.h"

namespace Utils::Process
{
	bool Run(const std::wstring& commandLine, const std::filesystem::path& workingDirectory, int* outExitCode)
	{
		// Create mutable buffer for CreateProcessW
		std::wstring cmd = commandLine;
		STARTUPINFOW si{};
		si.cb = sizeof(si);
		PROCESS_INFORMATION pi{};

		// convert working dir to wide string or pass nullptr
		LPCWSTR lpWorkingDir = nullptr;
		std::wstring workingDirW;
		if (!workingDirectory.empty())
		{
			workingDirW = workingDirectory.wstring();
			lpWorkingDir = workingDirW.c_str();
		}

		// CreateProcess: appName = null, cmdline mutable
		if (!CreateProcessW(
			nullptr,
			cmd.data(),
			nullptr,
			nullptr,
			FALSE,
			CREATE_NO_WINDOW,
			nullptr,
			lpWorkingDir,
			&si,
			& pi))
		{
			DWORD err = GetLastError();
			Logger::Log("CreateProcessW failed. Error: " + std::to_string(err), "Process", LOG_TYPE::CRITICAL);
			return false;
		}

		// wait until finished
		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		if (outExitCode)
			*outExitCode = static_cast<int>(exitCode);

		return true;
	}
	Result RunAndCapture(const std::wstring& cmdLine, const std::filesystem::path& workingDirectory)
	{
		Result result{};

		SECURITY_ATTRIBUTES sa{};
		sa.nLength = sizeof(sa);
		sa.bInheritHandle = TRUE;

		HANDLE hRead = nullptr;
		HANDLE hWrite = nullptr;

		if (!CreatePipe(&hRead, &hWrite, &sa, 0))
		{
			Logger::Log("CreatePipe Failed", "Process", LOG_TYPE::CRITICAL);
			return result;
		}

		// ensure read handle is not inherited
		SetHandleInformation(hRead, HANDLE_FLAG_INHERIT, 0);

		STARTUPINFOW si{};
		si.cb = sizeof(si);
		si.dwFlags = STARTF_USESTDHANDLES;
		si.hStdOutput = hWrite;
		si.hStdError = hWrite;
		si.hStdInput = nullptr;

		PROCESS_INFORMATION pi{};

		std::wstring mutableCmd = cmdLine;
		std::wstring workingDirW = workingDirectory.wstring();

		BOOL ok = CreateProcessW(
			nullptr,
			mutableCmd.data(), // must be mutable
			nullptr,
			nullptr,
			TRUE, // inherit handles
			CREATE_NO_WINDOW,
			nullptr,
			workingDirW.empty() ? nullptr : workingDirW.c_str(),
			&si,
			&pi
		);

		// parent no longer needs write end
		CloseHandle(hWrite);

		if (!ok)
		{
			DWORD err = GetLastError();
			Logger::Log("CreateProcessW failed: " + std::to_string(err), "Process", LOG_TYPE::CRITICAL);
			CloseHandle(hRead);
			return result;
		}

		// read output
		constexpr DWORD bufferSize = 4096;
		char buffer[bufferSize];
		DWORD bytesRead = 0;

		while (true)
		{
			BOOL success = ReadFile(hRead, buffer, bufferSize - 1, &bytesRead, nullptr);

			if (!success || bytesRead == 0)
				break;

			buffer[bytesRead] = '\0';
			result.Output += buffer;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exitCode = 0;
		GetExitCodeProcess(pi.hProcess, &exitCode);

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		CloseHandle(hRead);

		result.ExitCode = static_cast<int>(exitCode);
		result.Sucess = (exitCode == 0);

		return result;
	}
}