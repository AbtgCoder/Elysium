#pragma once

#include <string>

// Only windows for now
#include <Windows.h>
#include <commdlg.h>

class WindowsFileUtils
{
public:
	// return empty string if cancelled
	static std::string OpenFile(HWND hwnd, const char* filter);
	static std::string SaveFile(HWND hwnd, const char* filter);
};