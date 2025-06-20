#pragma once

#include <string>

// Only windows for now
#include <Windows.h>
#include <commdlg.h>
#include <ShlObj.h>

class WindowsFileUtils
{
public:
	// return empty string if cancelled
	static std::string OpenFile(const char* filter);
	static std::string SaveFile(const char* filter);
	static std::string OpenFolder();
};