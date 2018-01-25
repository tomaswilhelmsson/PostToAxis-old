#pragma once
#include <string>
#include <fstream>

namespace IniFile
{
	bool isOk();

	void setFile(const std::string path, std::string fileName);

	std::string getLocalAppData();

	std::string getString(const std::string section, const std::string key);
	void setString(const std::string section, const std::string key, const std::string value);

	int getInteger(const std::string section, const std::string key);
	void setInteger(const std::string section, const std::string key, int value);

}
