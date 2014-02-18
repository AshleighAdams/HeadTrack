#include "Options.h"
#include "Configor.h"
#include "Engine.h"
#include <sstream>

using namespace std;

COptions::COptions()
{
}

void COptions::LoadSettings()
{
	m_Config.LoadFromFile(pEngineInstance->GetFileSystem()->GetConfig("options.cfg"));
}

void COptions::SaveSettings()
{
	m_Config.SaveToFile(pEngineInstance->GetFileSystem()->GetConfig("options.cfg"));
}

IConfigorNode& COptions::GetSetting(const string& Name)
{
	string CurrentName = "";
	IConfigorNode* CurrentNode = m_Config.GetRootNode();
	char x;

	for(size_t i = 0; i < Name.length(); i++)
	{
		x = Name[i];
		if(x == '.')
		{
			CurrentNode = &(*CurrentNode)[CurrentName];
			CurrentName = "";
		}
		else if(x >= 'A' && x <= 'Z')
			CurrentName += x + 32;
		else
			CurrentName += x;
	}

	return (*CurrentNode)[CurrentName];
}

CConfigor& COptions::GetConfigor()
{
	return m_Config;
}
