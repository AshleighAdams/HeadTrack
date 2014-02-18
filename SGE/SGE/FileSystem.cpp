#include "FileSystem.h"

using namespace std;

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

void ReplaceAll(std::string& str, const std::string& from, const std::string& to)
{
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

CFileSystem::CFileSystem()
{
	m_BaseDIR = "";
	LoadedBaseDIR = false;
}

string CFileSystem::GetBaseDIR()
{
	if(!LoadedBaseDIR)
	{
		LoadedBaseDIR = true;
		char buffer[MAX_PATH];
		GetModuleFileName( NULL, buffer, MAX_PATH );

		string str = string(buffer);
		ReplaceAll(str, "\\", "/");
	
		string::size_type pos = str.find_last_of( "/" );
		m_BaseDIR = str.substr(0, pos + 1);
	}
	return m_BaseDIR;
}

string CFileSystem::GetSound(const string& File)
{
	return GetBaseDIR() + "sounds/" + File;
}

string CFileSystem::GetModel(const string& File)
{
	return GetBaseDIR() + "models/" + File;
}

string CFileSystem::GetTexture(const string& File)
{
	return GetBaseDIR() + "textures/" + File;
}

string CFileSystem::GetConfig(const string& File)
{
	return GetBaseDIR() + "config/" + File;
}

string CFileSystem::GetShader(const string& File)
{
	return GetBaseDIR() + "shaders/" + File;
}

string CFileSystem::GetScript(const string& File)
{
	return GetBaseDIR() + "lua/" + File;
}