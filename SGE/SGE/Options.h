#ifndef OPTIONS_H
#define OPTIONS_H

#include "Configor.h"

class COptions
{
public:
	COptions();
	void SaveSettings();
	void LoadSettings();
	IConfigorNode& GetSetting(const std::string& Name);
	CConfigor& GetConfigor();
private:
	CConfigor m_Config;
};



#endif