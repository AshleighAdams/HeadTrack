#include "Shader.h"
#define WIN32_LEAN_AND_MEAN

#include "Engine.h"

#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <Shellapi.h>

using namespace std;

CPreProcessor::CPreProcessor()
{
}
CPreProcessor::~CPreProcessor()
{
}

bool CPreProcessor::BuildShader(const std::string& file, std::string& out)
{
	string app = pEngineInstance->GetFileSystem()->GetBaseDIR() + "lupo.exe";
	LPCTSTR lpApplicationName = (LPCTSTR)app.c_str();

	STARTUPINFO lpStartupInfo;
	PROCESS_INFORMATION  lpProcessInfo;

	memset(&lpStartupInfo, 0, sizeof(lpStartupInfo));
	memset(&lpProcessInfo, 0, sizeof(lpProcessInfo));

	lpStartupInfo.cb = sizeof(lpStartupInfo);
	
	string commandline = "";

	if(m_Defines.size() > 0)
	{
		// set the defines
		for(auto it = m_Defines.begin(); it != m_Defines.end(); it++)
		{
			commandline += "-d" + (*it).first;
			commandline += "=";
			commandline += "\"" + (*it).second + "\" ";
		}
	}

	commandline += " -r \"" + pEngineInstance->GetFileSystem()->GetShader(file) + "\"";

	string env = pEngineInstance->GetFileSystem()->GetShader("");

	SetEnvironmentVariable((LPCSTR)"LUPO_INCLUDEPATH", (LPCSTR)env.c_str());
	SetCurrentDirectory((LPCSTR)env.c_str());

	DeleteFile("lupo.out");

	ShellExecute(0, 0, lpApplicationName, (LPSTR)commandline.c_str(), 0, SW_SHOWNORMAL);
	/*
	if (!CreateProcess(lpApplicationName, (LPSTR)commandline.c_str(), 0, 0, true, 0, 0, 0, &lpStartupInfo, &lpProcessInfo))
	{
		DWORD err = GetLastError();
		return false;
	}*/

	WaitForSingleObject(lpProcessInfo.hProcess, INFINITE);
	CloseHandle(lpProcessInfo.hProcess);
	CloseHandle(lpProcessInfo.hThread);
	
	double t = pEngineInstance->GetTime();
	while(pEngineInstance->GetTime() - t < 0.2);

	ifstream ifs("lupo.out");

	if(!ifs.is_open())
		return false;

	ifs.seekg(0, ios::end);
	size_t len = ifs.tellg();
	ifs.seekg(0, ios::beg);

	char* in = new char[len+1];

	for(size_t i = 0; i < len + 1; i++)
		in[i] = 0;

	ifs.read(in, len);
	out = string(in);

	delete [] in;
	return true;
}

void CPreProcessor::Define(const std::string& Def, const std::string& What)
{
	m_Defines[Def] = What;
}

void CPreProcessor::Define(const std::string& Def)
{
	Define(Def, "1");
}

void CPreProcessor::Undefine(const std::string& Def)
{
	m_Defines.erase(Def);
}

CShader::CShader()
{
	m_FSOID = m_VSOID = m_Program = 0;
}

CShader::~CShader()
{

}

bool CShader::Compile(const std::string& File, std::string& Error)
{
	m_Shader = File;

	m_PreProcessor.Define("VERTEX");
	if(!m_PreProcessor.BuildShader(m_Shader, m_VertexShader))
		return false;
	m_PreProcessor.Undefine("VERTEX");
	
	m_PreProcessor.Define("FRAGMENT");
	if(!m_PreProcessor.BuildShader(m_Shader, m_FragmentShader))
		return false;
	m_PreProcessor.Undefine("FRAGMENT");

#define assert_shader(Reason) \
	do \
	{ \
		unsigned int error = glGetError();\
		if(error)\
		{\
			cout << "Warning: Failed to compile shader " << File << " (" Reason ")\n";\
			return false;\
		}\
	} while(false);

	

#define assert_compile(ShaderObject) \
do \
{ \
	GLint compiled;\
	glGetObjectParameterivARB(m_VSOID, GL_COMPILE_STATUS, &compiled);\
	if(!compiled)\
	{\
		std::cout << "Failed to compile shader!\n";\
		GLint blen = 0; \
		GLsizei slen = 0; \
		glGetShaderiv(ShaderObject, GL_INFO_LOG_LENGTH, &blen); \
		if(blen > 1) \
		{\
			char* compiler_log = new char[blen]; \
			glGetShaderInfoLog(ShaderObject, blen, &slen, compiler_log); \
			std::cout << "Compile log: \n" << compiler_log << "\n"; \
			delete [] compiler_log; \
		}\
	}\
} while(false);

	m_VSOID = glCreateShader(GL_VERTEX_SHADER);
	assert_shader("glCreateShader vertex");
	
	m_FSOID = glCreateShader(GL_FRAGMENT_SHADER);
	assert_shader("glCreateShader fragment");

	m_VertexShader = "#version 120\n" + m_VertexShader;
	m_FragmentShader = "#version 120\n" + m_FragmentShader;
	const char* vs = m_VertexShader.c_str();
	const char* fs = m_FragmentShader.c_str();

	glShaderSource(m_VSOID, 1, &vs, 0);
	assert_shader("glShaderSource vertex");
	glShaderSource(m_FSOID, 1, &fs, 0);
	assert_shader("glShaderSource fragment");

	glCompileShader(m_VSOID);
	assert_shader("glCompileShader vertex");
	glCompileShader(m_FSOID);
	assert_shader("glCompileShader fragment");

	assert_compile(m_VSOID);
	assert_compile(m_FSOID);

	m_Program = glCreateProgram();
	assert_shader("glCreateProgram");

	glAttachShader(m_Program, m_VSOID);
	assert_shader("glAttachShader vertex");
	glAttachShader(m_Program, m_FSOID);
	assert_shader("glAttachShader fragment");

	glLinkProgram(m_Program);
	assert_shader("glLinkProgram");

	return true;
}


CPreProcessor* CShader::GetPreProcessor()
{
	return &m_PreProcessor;
}

bool CShader::SetUniform(const std::string& Name, float Value)
{
	int v = glGetUniformLocation(m_Program, Name.c_str());
	if(v > 1)
		return false;
	glUniform1f(v, Value);
	return true;
}

bool CShader::SetUniform(const std::string& Name, double Value)
{
	int v = glGetUniformLocation(m_Program, Name.c_str());
	if(v < 0)
		return false;
	glUniform1d(v, Value);
	return true;
}

bool CShader::SetUniform(const std::string& Name, int Value)
{
	int v = glGetUniformLocation(m_Program, Name.c_str());
	if(v < 0)
		return false;
	glUniform1i(v, Value);
	return true;
}

bool CShader::SetUniform(const std::string& Name, CVector& Value)
{
	int v = glGetUniformLocation(m_Program, Name.c_str());
	if(v < 0)
		return false;
	glUniform3d(v, Value.X, Value.Y, Value.Z);
	return true;
}

void CShader::Call()
{
	glUseProgram(m_Program);
}

void CShader::DrawQuad()
{
	glBegin(GL_QUADS);
		glVertex2d(-1, -1);
		glVertex2d(+1, -1);
		glVertex2d(+1, +1);
		glVertex2d(-1, +1);
	glEnd();
}

// TODO:  Make it so Enable and Disable don't call OpenGL functions, shades will be controlled by the world...

void CShader::Enable()
{
	glUseProgram(m_Program);
}

void CShader::Disable()
{
	glUseProgram(0);
}