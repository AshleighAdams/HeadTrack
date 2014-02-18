#define private public
#include "Engine.h"
#undef private

#include <ctime>

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_ACTIVATE:
		{
			pEngineInstance->m_Focused = !HIWORD(wParam);
			return 0;
		}
	case WM_SYSCOMMAND:
		{
			switch (wParam)
			{
			case SC_SCREENSAVE:
			case SC_MONITORPOWER:
				return false;
			}
			break;
		}
	case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

	case WM_KEYDOWN:
		pEngineInstance->m_KeyStates[wParam] = true;
		break;
	case WM_KEYUP:
		pEngineInstance->m_KeyStates[wParam] = false;
		return 0;

	case WM_SIZE:
		{
			pEngineInstance->UpdateViewport(LOWORD(lParam), HIWORD(lParam));
			return 0;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


CEngineInstance::CEngineInstance()
{
	m_LastSimulateTime = 0.0;
	m_LastTickTime = 0.0;
	m_CursorDepth = 0;

	m_FileSystem = CFileSystem();
	
	
	
	//m_Options.LoadSettings();
	
}

CEngineInstance::~CEngineInstance()
{
}

void CEngineInstance::Init()
{
	bool err = m_Options.GetConfigor().LoadFromFile(pEngineInstance->GetFileSystem()->GetConfig("options.cfg"));
	if(err)
		cout << m_Options.GetConfigor().GetError();
}

bool CEngineInstance::UpdateWindow(string Title, unsigned int Width, unsigned int Height, unsigned int ColorBit, bool Fullscreen)
{
	if(Width == 0 && Height == 0)
	{
		Width = 1200;
		Height = (double)Width * 9.0 / 16.0;
		//Width = pEngineInstance->GetOptions()->GetSetting("graphics.resolution.x").GetValue<unsigned int>(1920);
		//Height = pEngineInstance->GetOptions()->GetSetting("graphics.resolution.y").GetValue<unsigned int>(1080);
	}

	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left		= 0;
	WindowRect.right	= Width;
	WindowRect.top		= 0;
	WindowRect.bottom	= Height;

	m_FullScreen = Fullscreen;

	m_Instance			= GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc		= (WNDPROC) WndProc;					// WndProc Handles Messages TODO: This func doesn't exist!
	wc.cbClsExtra		= 0;									// No Extra Window Data
	wc.cbWndExtra		= 0;									// No Extra Window Data
	wc.hInstance		= m_Instance;							// Set The Instance
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground	= NULL;									// No Background Required For GL
	wc.lpszMenuName		= NULL;									// We Don't Want A Menu
	wc.lpszClassName	= "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL,"Failed To Register The Window Class.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}
	
	if (m_FullScreen)
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth	= Width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight	= Height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel	= ColorBit;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
		{
			if (MessageBox(NULL,"The requested fullscreen mode is not supported by\nyour video card. Use windowed mode instead?", "OpenGL", MB_YESNO | MB_ICONEXCLAMATION) != IDYES)
				return false;
			m_FullScreen = false;
		}
	}

	if (m_FullScreen)
	{
		dwExStyle=WS_EX_APPWINDOW;
		dwStyle=WS_POPUP;
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle=WS_OVERLAPPEDWINDOW;
	}

	ShowCursor(m_CursorDepth > 1);
	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	if (!(m_Wnd=CreateWindowEx(	dwExStyle,							// Extended Style For The Window
								"OpenGL",							// Class Name
								Title.c_str(),						// Window Title
								dwStyle |							// Defined Window Style
								WS_CLIPSIBLINGS |					// Required Window Style
								WS_CLIPCHILDREN,					// Required Window Style
								0, 0,								// Window Position
								WindowRect.right-WindowRect.left,	// Calculate Window Width
								WindowRect.bottom-WindowRect.top,	// Calculate Window Height
								NULL,								// No Parent Window
								NULL,								// No Menu
								m_Instance,							// Instance
								NULL)))								// Don't Pass Anything To WM_CREATE
	{
		KillWindow();
		MessageBox(NULL,"Window Creation Error.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	static	PIXELFORMATDESCRIPTOR pfd=				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		ColorBit,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		32,											// Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};
	
	if (!(m_DC=GetDC(m_Wnd)))
	{
		KillWindow();
		MessageBox(NULL,"Can't Create A GL Device Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if (!(PixelFormat=ChoosePixelFormat(m_DC, &pfd)))
	{
		KillWindow();
		MessageBox(NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if(!SetPixelFormat(m_DC, PixelFormat, &pfd))
	{
		KillWindow();
		MessageBox(NULL,"Can't Set The PixelFormat.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if (!(m_RC=wglCreateContext(m_DC)))
	{
		KillWindow();
		MessageBox(NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	if(!wglMakeCurrent(m_DC, m_RC))
	{
		KillWindow();
		MessageBox(NULL,"Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	ShowWindow(m_Wnd, SW_SHOW);
	SetForegroundWindow(m_Wnd);
	SetFocus(m_Wnd);
	UpdateViewport(Width, Height);

	m_WindowWidth	= Width;
	m_WindowHeight	= Height;

	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glCullFace(GL_FRONT);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	// TODO: REMOVE ME
	{
			// Set lighting intensity and color
		GLfloat qaAmbientLight[]    = {1.0, 1.0, 1.0, 1.0};
		GLfloat qaDiffuseLight[]    = {0.8, 0.8, 0.8, 1.0};
		GLfloat qaSpecularLight[]    = {1.0, 1.0, 1.0, 1.0};
		GLfloat emitLight[] = {0.9, 0.9, 0.9, 0.01};
		GLfloat Noemit[] = {0.0, 0.0, 0.0, 1.0};
			// Light source position
		
		glEnable(GL_LIGHT0);

		// Set lighting intensity and color
		glLightfv(GL_LIGHT0, GL_AMBIENT, qaAmbientLight);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, qaDiffuseLight);
		glLightfv(GL_LIGHT0, GL_SPECULAR, qaSpecularLight);
	
		
	}
	auto err = glewInit();
	if(err != GLEW_OK)
	{
		cout << glewGetErrorString(err);
		return false;
	}

	// Lets turn vsync off...

	BOOL (APIENTRY *wglSwapIntervalEXT)(int);

	const char *extensions = (char*)glGetString( GL_EXTENSIONS );

	if( strstr( extensions, "WGL_EXT_swap_control" ) != 0 )
	{
		wglSwapIntervalEXT = (BOOL(APIENTRY *)(int))wglGetProcAddress( "wglSwapIntervalEXT" );
		if( wglSwapIntervalEXT )
			wglSwapIntervalEXT(true); // Turn vsync on
	}
	
	return true;
}

bool CEngineInstance::Focused()
{
	return GetActiveWindow() == m_Wnd;
}

void CEngineInstance::KillWindow()
{
	if (m_FullScreen)
		ChangeDisplaySettings(0, 0);

	ShowCursor(true);

	if (m_RC)
	{
		if (!wglMakeCurrent(0, 0))
			MessageBox(NULL,"Release Of DC And RC Failed." , "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);

		if (!wglDeleteContext(m_RC))
		{
			MessageBox(NULL, "Release Rendering Context Failed.","SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		m_RC = 0;
	}

	if (m_DC && !ReleaseDC(m_Wnd, m_DC))
	{
		MessageBox(NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK | MB_ICONINFORMATION);
		m_DC = 0;
	}

	if (m_Wnd && !DestroyWindow(m_Wnd))
	{
		MessageBox(0, "Could Not Release m_Wnd.","SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		m_Wnd = 0;
	}

	if (!UnregisterClass("OpenGL", m_Instance))
	{
		MessageBox(0, "Could Not Unregister Class.","SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		m_Instance = 0;
	}
}

void CEngineInstance::UpdateViewport(unsigned int Width, unsigned int Height)
{
	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	double fov = 90; //m_Options.GetSetting("graphics.fov").GetValue<double>(90.0);
	gluPerspective(fov / 2.f, (GLfloat)Width / (GLfloat)Height, 0.1f, 10000.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CEngineInstance::Update()
{
	double ct = GetTime();

	const double tick_time = 1.0 / 66.6;

	if(ct - m_LastTickTime > tick_time)
	{
		m_World.Think();
		m_LastTickTime += (ct - m_LastTickTime); // More accurate, you will get 66 ticks over 1 second instead of, say 68 or 64
	}

	m_World.Simulate(ct - m_LastSimulateTime);
	m_Camera.Update(ct - m_LastSimulateTime);
	m_LastSimulateTime = ct;
}

void CEngineInstance::Draw()
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glLoadIdentity();
	
	glPushMatrix();
	m_Camera.LookAt();
	
	for(float i = -100; i <= 100; i++)
	{
		glBegin(GL_LINES); // well okay its not that bad
			glColor3ub(50, 50, 50);						
			glVertex3d(-100, i, 0);					
			glVertex3d(100, i, 0);
			glVertex3d(i, -100,0);							
			glVertex3d(i, 100, 0);
		glEnd();
	}
	
	m_World.DrawDebug();
	
	
	GLfloat qaLightPosition[]    = {10.5, 0, 0.5, 0.5};
	glLightfv(GL_LIGHT0, GL_POSITION, qaLightPosition);
	glEnable(GL_LIGHTING);
		m_World.Draw();
	glDisable(GL_LIGHTING);
	
	glPopMatrix();

	m_World.PostDraw();
}

void CEngineInstance::SwapBuffer()
{
	SwapBuffers(m_DC);
}

CCamera* CEngineInstance::GetCamera()
{
	return &m_Camera;
}

CFileSystem* CEngineInstance::GetFileSystem()
{
	return &m_FileSystem;
}

COptions* CEngineInstance::GetOptions()
{
	return &m_Options;
}

CWorld* CEngineInstance::GetWorld()
{
	return &m_World;
}

double CEngineInstance::GetTime()
{
	static LARGE_INTEGER freq, start;
	LARGE_INTEGER count;
	if (!QueryPerformanceCounter(&count))
		return -1;
	if (!freq.QuadPart) 
	{
		if (!QueryPerformanceFrequency(&freq))
			return -1;
		start = count;
	}
	return (double)(count.QuadPart - start.QuadPart) / freq.QuadPart;
}

