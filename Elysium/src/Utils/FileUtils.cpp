#include "FileUtils.h"

std::string WindowsFileUtils::OpenFile(HWND hwnd, const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd; // win32 handle of parent window
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // include NOCHANGEDIR flag else some issues are encountered
	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}

	return std::string();
}

std::string WindowsFileUtils::SaveFile(HWND hwnd, const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd; // win32 handle of parent window
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR; // include NOCHANGEDIR flag else some issues are encountered
	ofn.lpstrDefExt = strchr(filter, '\0') + 1; // set file extension by extracting it from the filter
	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		return ofn.lpstrFile;
	}
	return std::string();
}
