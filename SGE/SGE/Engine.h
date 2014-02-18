#ifndef ENGINE_H
#define ENGINE_H

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glew32.lib")

#include <windows.h>
#include <GL/glew.h>
/*
#include <gl/gl.h>
#include <gl/glu.h>
*/

#include <string>
#include <sstream>

#include "Camera.h"
#include "FileSystem.h"
#include "Options.h"
#include "Model.h"
#include "World.h"
#include "Shader.h"
#include "Texture.h"

#define sge_assert(_Type, _What) \
	do\
	{\
		_Type what = (_What);\
		if(!(what)) \
		{ \
			std::stringstream ss;\
			ss << "Assertion failed at ";\
			ss << __FILE__ << ":" << __LINE__ << " | ";\
			ss << (#_What) << " = " << what;\
			\
			std::cout << ss.str() << "\n";\
		} \
	} while(false)

class CModel;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

class CEngineInstance
{
public:
	CEngineInstance();
	~CEngineInstance();
	bool UpdateWindow(std::string Title, unsigned int Width, unsigned int Height, unsigned int ColorBit, bool Fullscreen);
	void KillWindow();
	void UpdateViewport(unsigned int Width, unsigned int Height);
	void Init();
	void Update();
	void Draw();
	void SwapBuffer();
	double GetTime();
	bool Focused();
	CCamera*		GetCamera();
	CFileSystem*	GetFileSystem();
	COptions*		GetOptions();
	CWorld*			GetWorld();
//private:
	CCamera		m_Camera;
	CFileSystem m_FileSystem;
	COptions	m_Options;
	CWorld 		m_World;

	double m_LastSimulateTime;
	double m_LastTickTime;
	bool m_Focused;
	bool m_FullScreen;
	bool m_KeyStates[256];
	unsigned int m_WindowWidth;
	unsigned int m_WindowHeight;

	unsigned int m_CursorDepth;

	/* Windows specific */
	HDC			m_DC;
	HGLRC		m_RC;
	HWND		m_Wnd;
	HINSTANCE	m_Instance;
	/* End */
};

extern CEngineInstance* pEngineInstance;

#endif