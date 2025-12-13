#pragma once

#include <VyLib/VyLib.h>
#include <VyLib/STL/Path.h>

#include <windows.h>
#include <commdlg.h>
#include <locale>
#include <fstream>

typedef DWORD PlatformFlags;
typedef LPCWSTR NPSString; //Non-Platform-Specific String

namespace Vy 
{
	namespace Platform
	{
		static String convertWideToUtf8(const wchar_t* wideString) 
        {
			if (wideString == nullptr) 
            {
				return String();
			}

			int size = WideCharToMultiByte(CP_UTF8, 0, wideString, -1, NULL, 0, NULL, NULL);
			if (size == 0) 
            {
				return String();
			}

			String utf8String(size, 0);
			WideCharToMultiByte(CP_UTF8, 0, wideString, -1, &utf8String[0], size, NULL, NULL);

			utf8String.resize(size - 1);

			return utf8String;
		}


		static WString convertUtf8ToWide(const String& string) 
        {
			if (string.empty()) 
            {
				return WString();
			}

			int size = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), (int)string.size(), NULL, 0);
			WString wideStr(size, 0);
			MultiByteToWideChar(CP_UTF8, 0, string.c_str(), (int)string.size(), &wideStr[0], size);

			return wideStr;
		}


		static bool isChildOf(const Path& parent, const Path& child) 
        {
			try 
            {
				Path relativePath = FS::relative(child, parent);
				return !relativePath.empty() && relativePath.has_root_path() == false;
			}
			catch (const FS::filesystem_error& e) 
            {
				VY_ERROR("isChildOf Filesystem error: {0} ", e.what());
				return false;
			}
		}


		// static int showMessageBox(String title, String message, PlatformFlags flags = MB_OK, bool blockInput = true)
		// {
		// 	WString _title   = Platform::convertUtf8ToWide(title);
		// 	WString _message = Platform::convertUtf8ToWide(title);
		// 	HWND    hWnd     = NULL;

		// 	if (blockInput)
		// 	{
		// 		hWnd = GetConsoleWindow();
		// 	}

		// 	return MessageBox(hWnd, _title.c_str(), _message.c_str(), flags);
		// }


		// static int showMessageBox(WString title, WString message, PlatformFlags flags = MB_OK, bool blockInput = true)
		// {
		// 	HWND hWnd = NULL;

		// 	if (blockInput)
		// 	{
		// 		hWnd = GetConsoleWindow();
		// 	}

		// 	return MessageBox(hWnd, title.c_str(), message.c_str(), flags);
		// }


		// //Open File dialog
		// static bool openFileDialog(Path& filePath, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) 
        // {
		// 	Path workingDirectory = FS::current_path();
		// 	VY_WARN("File extension is not specified! Not specifying file extension is not recommended!");
		// 	VY_WARN("File filter is not specified! Not specifying file filter is not recommended!");
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = L"All Files\0*.*\0";
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.Flags = flags;

		// 	if (GetOpenFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File opened: {0}", Platform::convertWideToUtf8(_filePath));
		// 		FS::current_path(workingDirectory);
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		FS::current_path(workingDirectory);
		// 		return false;
		// 	}
		// }

		// //Open File dialog
		// static bool openFileDialog(Path& filePath, NPSString fileExtension, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) 
        // {
		// 	Path workingDirectory = FS::current_path();
		// 	VY_WARN("File filter is not specified! Not specifying file filter is not recommended!");
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = L"All Files\0*.*\0";
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.lpstrDefExt = fileExtension;
		// 	ofn.Flags = flags;

		// 	if (GetOpenFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File opened: {0}", Platform::convertWideToUtf8(_filePath));
		// 		FS::current_path(workingDirectory);
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		FS::current_path(workingDirectory);
		// 		return false;
		// 	}
		// }


		// //Open File dialog
		// static bool openFileDialog(Path& filePath, NPSString fileExtension, NPSString filter, PlatformFlags flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY) 
        // {
		// 	Path workingDirectory = FS::current_path();
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = filter;
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.lpstrDefExt = fileExtension;
		// 	ofn.Flags = flags;

		// 	if (GetOpenFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File opened: {0}", Platform::convertWideToUtf8(_filePath));
		// 		FS::current_path(workingDirectory);
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		FS::current_path(workingDirectory);
		// 		return false;
		// 	}
		// }


		// //Save File dialog
		// static bool saveFileDialog(Path& filePath, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT) 
        // {
		// 	VY_WARN("File extension is not specified! Not specifying file extension is not recommended!");
		// 	VY_WARN("File filter is not specified! Not specifying file filter is not recommended!");
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = L"All Files\0*.*\0";
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.Flags = flags;

		// 	if (GetSaveFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File saved: {0}", Platform::convertWideToUtf8(_filePath));
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }


		// //Save File dialog
		// static bool saveFileDialog(Path& filePath, NPSString fileExtension, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT) 
        // {
		// 	VY_WARN("File filter is not specified! Not specifying file filter is not recommended!");
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = L"All Files\0*.*\0";
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.lpstrDefExt = fileExtension;
		// 	ofn.Flags = flags;

		// 	if (GetSaveFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File saved: {0}", Platform::convertWideToUtf8(_filePath));
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }


		// //Save File dialog
		// static bool saveFileDialog(Path& filePath, NPSString fileExtension, NPSString filter, PlatformFlags flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT)
		// {
		// 	WCHAR _filePath[MAX_PATH];
		// 	OPENFILENAME ofn;
		// 	ZeroMemory(&ofn, sizeof(ofn));
		// 	ofn.lStructSize = sizeof(ofn);
		// 	ofn.hwndOwner = GetConsoleWindow();
		// 	ofn.lpstrFilter = filter;
		// 	ofn.lpstrFile = _filePath;
		// 	ofn.lpstrFile[0] = '\0';
		// 	ofn.nMaxFile = MAX_PATH;
		// 	ofn.lpstrDefExt = fileExtension;
		// 	ofn.Flags = flags;

		// 	if (GetSaveFileName(&ofn)) 
        //     {
		// 		filePath = Path(WString(_filePath));
		// 		VY_INFO("File saved: {0}", Platform::convertWideToUtf8(_filePath));
		// 		return true;
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }

#ifdef createDirectory
    #undef createDirectory
#endif

		static bool createDirectory(const Path path) 
		{
			try {
				if (FS::create_directory(path)) 
                {
					VY_INFO("Directory created: {0}", path.string());
					return true;
				}
				else 
                {
					VY_ERROR("Failed to create directory (no specific reason provided): {0}", path.string());
					return false;
				}
			}
			catch (const FS::filesystem_error& e) 
            {
				VY_ERROR("Error: {0}", e.what());
				return false;
			}
			catch (const std::exception& e) 
            {
				VY_ERROR("Error: {0}", e.what());
				return false;
			}
		}


		static bool createDirectory(const Path paths[])
		{
			if (paths->empty()) 
            {
				VY_ERROR("Paths array is empty!");
				return false;
			}
			for (size_t i = 0; i < (sizeof(paths)/sizeof(paths[0])); i++) 
            {
				if (!FS::create_directory(paths[i])) 
                {
					VY_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}
				VY_INFO("Directory created: {0}", paths[i].string());
			}
			return true;
		}


		static bool createDirectory(const TVector<Path> paths)
		{
			if (paths.empty()) 
            {
				VY_ERROR("Paths vector is empty!");
				return false;
			}

			for (size_t i = 0; i < paths.size(); i++) 
            {
				if (!FS::create_directory(paths[i])) 
                {
					VY_ERROR("Failed to create directory: {0}", paths[i].string());
					return false;
				}

				VY_INFO("Directory created: {0}", paths[i].string());
			}

			return true;
		}


		// static bool deleteDirectory(const Path path, String title = "Delete selected?", String message = "You cannot undo the delete action.")
		// {
		// 	WString _title   = Platform::convertUtf8ToWide(title);
		// 	WString _message = Platform::convertUtf8ToWide(message);
		// 	int messageBoxID = Platform::showMessageBox(_title, _message, MB_ICONQUESTION | MB_YESNO);
			
        //     if (messageBoxID == IDYES) 
        //     {
		// 		try 
        //         {
		// 			if (FS::remove_all(path)) 
        //             {
		// 				VY_INFO("Directory deleted successfully.");
		// 				return true;
		// 			}
		// 			else 
        //             {
		// 				VY_INFO("Unable to delete directory.");
		// 				return false;
		// 			}
		// 		}
		// 		catch (const FS::filesystem_error& e) 
        //         {
		// 			VY_ERROR("Error deleting directory: {0}", e.what());
		// 			return false;
		// 		}
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }


		// static bool deleteDirectory(const Path path, WString title = L"Delete selected?", WString message = L"You cannot undo the delete action.")
		// {
		// 	int messageBoxID = Platform::showMessageBox(title, message, MB_ICONQUESTION | MB_YESNO);

		// 	if (messageBoxID == IDYES) 
        //     {
		// 		try 
        //         {
		// 			if (FS::remove_all(path)) 
        //             {
		// 				VY_INFO("Directory deleted successfully.");
		// 				return true;
		// 			}
		// 			else 
        //             {
		// 				VY_INFO("Unable to delete directory.");
		// 				return false;
		// 			}
		// 		}
		// 		catch (const FS::filesystem_error& e) 
        //         {
		// 			VY_ERROR("Error deleting directory: {0}", e.what());
		// 			return false;
		// 		}
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }


		// static bool deleteDirectory(const Path path, int messageBoxID)
		// {
		// 	if (messageBoxID == IDYES) 
        //     {
		// 		try 
        //         {
		// 			if (FS::remove_all(path)) 
        //             {
		// 				VY_INFO("Directory deleted successfully.");
		// 				return true;
		// 			}
		// 			else 
        //             {
		// 				VY_INFO("Unable to delete directory.");
		// 				return false;
		// 			}
		// 		}
		// 		catch (const FS::filesystem_error& e) 
        //         {
		// 			VY_ERROR("Error deleting directory: {0}", e.what());
		// 			return false;
		// 		}
		// 	}
		// 	else 
        //     {
		// 		return false;
		// 	}
		// }


		static std::ofstream outputFile(const Path path) 
		{
			std::ofstream file(path);
			if (!file.is_open()) {
				Path parent = path.parent_path();
				if (!FS::exists(parent)) {
					VY_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					VY_ERROR("Failed to open/create file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good()) {
				VY_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else {
				VY_INFO("Output File: {0}", path.string());
				return file;
			}
		}

		static bool outputFile(std::ofstream& file, const Path path) 
		{
			file = std::ofstream(path);
			if (!file.is_open()) 
            {
				Path parent = path.parent_path();
				
                if (!FS::exists(parent)) 
                {
					VY_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else 
                {
					VY_ERROR("Failed to open/create file: {0}", path.string());
				}

				return false;
			}
			else if (!file.good()) 
            {
				VY_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else 
            {
				VY_INFO("Output File: {0}", path.string());
				return true;
			}
		}

		static std::ifstream openFile(const Path path) 
		{
			std::ifstream file(path);
			if (!file.is_open()) 
            {
				Path parent = path.parent_path();
				if (!FS::exists(parent)) 
                {
					VY_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else 
                {
					VY_ERROR("Failed to open file: {0}", path.string());
				}
				return file;
			}
			else if (!file.good()) 
            {
				VY_ERROR("File is not in good state: {0}", path.string());
				return file;
			}
			else 
            {
				VY_INFO("Opened File: {0}", path.string());
				return file;
			}
		}

		static bool openFile(std::ifstream& file, const Path path)
		{
			file = std::ifstream(path);
			if (!file.is_open()) {
				Path parent = path.parent_path();
				if (!FS::exists(parent)) {
					VY_ERROR("Parent directory does not exist: {0}", parent.string());
				}
				else {
					VY_ERROR("Failed to open file: {0}", path.string());
				}
				return false;
			}
			else if (!file.good()) {
				VY_ERROR("File is not in good state: {0}", path.string());
				return false;
			}
			else {
				VY_INFO("Opened File: {0}", path.string());
				return true;
			}
		}
	}
}