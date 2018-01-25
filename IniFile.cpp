#include "ShlObj.h"

#include <Windows.h>

#include "IniFile.h"


namespace IniFile {
	bool _isOk;

	std::string _path;
	std::string _fileName;

	std::string _fullPath;

	std::fstream _fStream;


	bool isOk() {
		return _isOk;
	}

	void setFile(const std::string path, std::string fileName)
	{
		_path = path;
		_fileName = fileName;

		_fullPath = path + "\\" + fileName;

		int ret = ::CreateDirectory(path.c_str(), NULL);

		if (ret != 0) // Directory created
		{
			// Create empty file
			std::fstream tmpFile;
			tmpFile.open(path + "\\" + fileName, std::ios::out);
			tmpFile.close();
			_isOk = true;
		}
		else {
			DWORD err = ::GetLastError();

			if (err == ERROR_ALREADY_EXISTS) // Path exists
			{
				if (
					(GetFileAttributes(_fullPath.c_str()) == INVALID_FILE_ATTRIBUTES) &&
					(GetLastError() == ERROR_FILE_NOT_FOUND)
					) // Filename not found create it
				{
					std::fstream tmpFile;
					tmpFile.open(path + "\\" + fileName, std::ios::out);
					tmpFile.close();
					setString("Settings", "IP", "0.0.0.0");
					setString("Settings", "PORT", "9992");
					_isOk = true;
				}
				else // File exists
					_isOk = true;
			}
			else if (err == ERROR_PATH_NOT_FOUND)
				_isOk = false;
		}
	}

	std::string getLocalAppData()
	{
		LPWSTR returnPath;

		HRESULT result = SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, nullptr, &returnPath);

		if (FAILED(result))
			return std::string(); // Return empty string if failed

		std::wstring wstr(returnPath);

		return std::string(wstr.begin(), wstr.end());
	}

	std::string getString(const std::string section, const std::string key)
	{
		if (_fullPath.empty())
			return "";

		char retString[64];

		::GetPrivateProfileString(section.c_str(), key.c_str(), "", retString, 64, _fullPath.c_str());

		return std::string(retString);
	}

	void setString(const std::string section, const std::string key, const std::string value)
	{
		if (_fullPath.empty())
			return;

		::WritePrivateProfileString(section.c_str(), key.c_str(), value.c_str(), _fullPath.c_str());
	}

	int getInteger(const std::string section, const std::string key)
	{
		if (_fullPath.empty())
			return 0;

		int retValue;

		retValue = ::GetPrivateProfileInt(section.c_str(), key.c_str(), 0, _fullPath.c_str());

		return retValue;
	}

	void setInteger(const std::string section, const std::string key, int value)
	{
		if (_fullPath.empty())
			return;

		std::string strValue = std::to_string(value);

		::WritePrivateProfileString(section.c_str(), key.c_str(), strValue.c_str(), _fullPath.c_str());
	}
}