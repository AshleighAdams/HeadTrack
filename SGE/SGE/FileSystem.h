#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <string>

class CFileSystem
{
public:
	CFileSystem();
	std::string GetBaseDIR();
	std::string GetSound(const std::string& File);
	std::string GetModel(const std::string& File);
	std::string GetTexture(const std::string& File);
	std::string GetConfig(const std::string& File);
	std::string GetShader(const std::string& File);
	std::string GetScript(const std::string& File);
private:
	std::string m_BaseDIR;
	bool LoadedBaseDIR;
};

#endif