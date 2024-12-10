#include "FileUtils.h"

#include "core/Logger.h"

#include <ShObjIdl.h>

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

std::string WindowsFileUtils::OpenFolder(HWND hwnd)
{
	BROWSEINFO bi = { 0 };
	bi.lpszTitle = L"Select a Folder";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;  // only shows folders...
	bi.hwndOwner = hwnd;

	PIDLIST_ABSOLUTE pidl = SHBrowseForFolder(&bi);

	if (pidl != nullptr)
	{
		wchar_t szFolderPath[MAX_PATH];
		if (SHGetPathFromIDList(pidl, szFolderPath))
		{
			CoTaskMemFree(pidl); // free the PIDL memory
			char folderPath[MAX_PATH];
			size_t numConverted;
			wcstombs_s(&numConverted, folderPath, szFolderPath, MAX_PATH);
			return std::string(folderPath);
		}
		CoTaskMemFree(pidl); // free the PIDL memory
	}

	return std::string();

#if 0
	// initialize com library
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
		Logger::Log("Couldn't Initialize com library", "editor", LOG_TYPE::CRITICAL);
		return std::string();

	// create file open dialog select
	IFileDialog* pFileDialog = nullptr;
	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFileDialog));
	if (FAILED(hr))
	{
		CoUninitialize();
		Logger::Log("CoCreateInstance failed", "editor", LOG_TYPE::CRITICAL);
		return std::string();
	}

	DWORD dwOptions;
	if (SUCCEEDED(pFileDialog->GetOptions(&dwOptions)))
	{
		pFileDialog->SetOptions(dwOptions | FOS_PICKFOLDERS); // "pick folders" option
	}

	// show dialog
	if (SUCCEEDED(hr))
	{
		// get the selected folder
		IShellItem* pItem = nullptr;
		hr = pFileDialog->GetResult(&pItem);
		if (SUCCEEDED(hr))
		{
			PWSTR pszFolderPath = nullptr;
			hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFolderPath);
			if (SUCCEEDED(hr))
			{

				// convert wide string to a standard string
				char folderPath[MAX_PATH];
				size_t numConverted;
				wcstombs_s(&numConverted, folderPath, pszFolderPath, MAX_PATH);

				// clean up
				CoTaskMemFree(pszFolderPath);
				pItem->Release();
				pFileDialog->Release();
				CoUninitialize();

				return std::string(folderPath);
			}
			pItem->Release();
		}
	}

	// clean up
	pFileDialog->Release();
	CoUninitialize();
	
	return std::string();
#endif
}
