#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <unordered_map>

#include "Vector.h"

class CPreProcessor
{
public:
	CPreProcessor();
	~CPreProcessor();
	void Define(const std::string& Def, const std::string& What);
	void Define(const std::string& Def);

	void Undefine(const std::string& Def);

	bool BuildShader(const std::string& file_in, std::string& out);
private:
	std::unordered_map<std::string, std::string> m_Defines;
};

class CShader
{
public:
	CShader();
	virtual ~CShader();
	virtual CPreProcessor* GetPreProcessor();
	virtual bool Compile(const std::string& File, std::string& Error);
	//virtual bool CreateProgram();
	virtual void Enable();
	virtual void Disable();
	virtual void Call();
	virtual void DrawQuad();

	virtual bool SetUniform(const std::string& Name, CVector& Value);
	virtual bool SetUniform(const std::string& Name, float Value);
	virtual bool SetUniform(const std::string& Name, double Value);
	virtual bool SetUniform(const std::string& Name, int Value);

protected:
	
	
	CPreProcessor m_PreProcessor;
	std::string m_Shader;

	int m_Program;

	int m_VSOID;
	int m_FSOID;

	std::string m_VertexShader;
	std::string m_FragmentShader;
	std::string m_GeometryShader;
};

#endif