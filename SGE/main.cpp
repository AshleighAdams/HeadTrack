
#include <cstdlib>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "SGE/Engine.h"
CEngineInstance* pEngineInstance = 0;

#include <iostream>
#include <string>
#include <functional>
#include <sstream>
#include <fstream>

#include <boost/thread.hpp>

using namespace std;

#include "SGE/FileSystem.h"
#include "SGE/Model.h"

#include <lua.hpp>
lua_State* g_pState = 0;
#pragma comment(lib, "lua51.lib")

struct TFreeTrackData
{
	int DataID;
	int CamWidth;
	int CamHeight;
	// virtual pose
	float Yaw;   // positive yaw to the left
	float Pitch; // positive pitch up
	float Roll;  // positive roll to the left
	float X;
	float Y;
	float Z;
	// raw pose with no smoothing, sensitivity, response curve etc.
	float RawYaw;
	float RawPitch;
	float RawRoll;
	float RawX;
	float RawY;
	float RawZ;
	// raw points, sorted by Y, origin top left corner
	float X1;
	float Y1;
	float X2;
	float Y2;
	float X3;
	float Y3;
	float X4;
	float Y4;
};

int GraphPos = 0;
CAngle RawInput[100];
CAngle SmoothOut[100];

class CTestShader : public CShader
{
public:
	CTestShader()
	{
		TimeU = 0;
	}
	virtual bool Compile(const std::string& File, std::string& Error)
	{
		bool ret = CShader::Compile(File, Error);
		Call();
		TimeU = glGetUniformLocation(m_Program, "time");
		sge_assert(bool, TimeU >= 0);
		return ret;
	}
	bool SetTime(double Val)
	{
		if(TimeU < 0)
			return false;
		glUniform1f(TimeU, Val);
		return true;
	}
private:
	int TimeU;
};

class CStaticProp : public CBaseEntity
{
public:
	string 		m_ClassName;
	CModel		m_Model;
	CVector		m_Pos;
	CAngle		m_Ang;
	CTestShader	m_Shader;
	CTexture	m_Texture;
public:
	CStaticProp()
	{
		string s = CBaseEntity::GetClass();
		m_ClassName = "CStaticProp:" + s;
		m_Model.SetModel("cube.obj", true);
		m_Pos = CVector(0, 0, 0);
		m_Ang = CAngle(0, 0, 0);
		m_Shader = CTestShader();
		string null;
		
		/*m_Shader.Compile("test.shader", null);
		m_Shader.Enable();
		m_Shader.SetTime(pEngineInstance->GetTime());
		m_Shader.Disable();
		*/
		m_Texture.LoadFromFile("player_chrome1.tga");

		
	}
	~CStaticProp()
	{
	}
	const char* GetClass()
	{
		return m_ClassName.c_str();
	}
	void SetPosition(const CVector& Vec)
	{
		m_Pos = Vec;
	}
	void SetModel(const string& str, bool SwapYZ = false)
	{
		m_Model.SetModel(str, SwapYZ);
	}
	void Think() // Called every tick
	{
	}
	void Simulate(double Time) // Called every frame, with a frame time
	{
		//m_Ang.Yaw += 40.0 * Time; // 10 Deg per second
	}
	void PostDraw()
	{
	}
	void Draw()
	{
		//m_Shader.Enable();
		//	m_Shader.SetTime(pEngineInstance->GetTime());
			glEnable(GL_TEXTURE_2D);
				glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				m_Texture.Bind();
				m_Model.Draw(m_Pos, m_Ang);
			glDisable(GL_TEXTURE_2D);
		//m_Shader.Disable();
	}
	void DrawDebug()
	{
		m_Model.DrawNormals(m_Pos, m_Ang);
	}
};

float log2(float n)
{  
	// log(n)/log(2) is log2.  
	return log(n) / log(2.f);  
}
class PID_Controller
{
public:
	void Initialize(double kp, double ki, double kd, double error_thresh);
	double Update(double val);

private:
	bool m_started;
	double m_kp, m_ki, m_kd, m_h, m_inv_h, m_prev_error, m_error_thresh, m_integral;
	double lasttime;
	double lastval;
};

void PID_Controller::Initialize(double kp, double ki, double kd, double error_thresh)
{
	lasttime = pEngineInstance->GetTime();
	lastval = 0;

	m_kp=kp;
	m_ki=ki;
	m_kd=kd;
	m_prev_error = 0;
	m_error_thresh=error_thresh;

	m_h = pEngineInstance->GetTime() - lasttime;
	m_h = 10;//1.0 / 1600000.0;

	m_h = m_h == 0 ? 0.0001 : m_h;
	m_inv_h=1 / m_h;

	m_integral=0;
	m_started=false;
}

double PID_Controller::Update(double val)
{
	double error = lastval - val;
	lastval = val;

	m_h = pEngineInstance->GetTime() - lasttime;
	m_h = 10;//1.0 / 1600000.0;
	m_h = m_h == 0 ? 0.0001 : m_h;
	m_inv_h = 1.0 / m_h;

	lasttime = pEngineInstance->GetTime();

	double q;
	if(fabs(error)<m_error_thresh)				//integrator windup
		q=1;
	else
		q=0;

	m_integral+=m_h*q*error;

	double deriv;
	if(!m_started)
	{
		m_started=true;
		deriv=0;
	}
	else deriv=(error-m_prev_error)*m_inv_h;
	
	m_prev_error=error;

	return val - (
		m_kp*(error+m_ki*m_integral+m_kd*deriv)
		);
}


float BSL(float in, float c)
{
	return in * pow(2.0f, c);
}

float BSR(float in, float c)
{
	return in / pow(2.0f, c);
}



class CEWMA
{
public:
	CEWMA(){}
	CEWMA(float factor, float weight)
	{
		m_Factor = log2(factor);
		m_Weight = log2(weight);
		m_Internal = 0;
	}

	void Set(float factor, float weight)
	{
		m_Factor = log2(factor);
		m_Weight = log2(weight);
	}

	void Add(float A)
	{
		if(m_Internal)
			m_Internal = BSR(
				BSL(m_Internal, m_Weight) - m_Internal + BSL(A, m_Factor)
			, m_Weight);
		else
			m_Internal = BSL(A, m_Factor);
	}

	float Get()
	{
		return BSR(m_Internal, m_Factor);
		//return m_Internal >> m_Factor;
	}
private:
	float m_Internal;
	float m_Factor;
	float m_Weight;

};

using boost::asio::ip::udp;

class server
{
public:
	server(boost::asio::io_service& io_service, short port, CStaticProp* pProp)
		: io_service_(io_service),
		socket_(io_service, udp::endpoint(udp::v4(), port))
	{
		
#define FACTOR_VAL 1024 * 1024
#define WEIGHT_VAL 64
		ewma_p = CEWMA(FACTOR_VAL, WEIGHT_VAL);
		ewma_y = CEWMA(FACTOR_VAL, WEIGHT_VAL);
		ewma_r = CEWMA(FACTOR_VAL, WEIGHT_VAL);
		
		kp = 1;
		kd = 0;
		ki = 0;

		PID_P.Initialize(kp, ki, kd, 100);
		PID_Y.Initialize(kp, ki, kd, 100);
		PID_R.Initialize(kp, ki, kd, 100);

		prop = pProp;
		retrack = true;
		model.DataID++;
		model.CamWidth = 320;
		model.CamHeight = 240;

		model.Yaw = 0;
		model.Pitch = 0;
		model.Roll = 0;
		model.X = 0;
		model.Y = 0;
		model.Z = 0;
		model.RawYaw = 0;
		model.RawPitch = 0;
		model.RawRoll = 0;
		model.RawX = 0;
		model.RawY = 0;
		model.RawZ = 0;
		model.X1 = 0;
		model.Y1 = 0;
		model.X2 = 0;
		model.Y2 = 0;
		model.X3 = 0;
		model.Y3 = 0;
		model.X4 = 0;
		model.Y4 = 0;

		hFileMap = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, "FT_SharedMem");

		if(!hFileMap)
			hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE, 0, sizeof(TFreeTrackData), "FT_SharedMem");

		if(!hFileMap)
		{
			cerr << "failed to open FT_SharedMem\n";
			return;
		}

		lpvSharedMemory = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFreeTrackData));

		if(!lpvSharedMemory)
		{
			cerr << "failed to map FT_SharedMem\n";
			return;
		}

		
		hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "FT_Mutext");
		if(!lpvSharedMemory)
			hMutex = CreateMutex(0, FALSE, "FT_Mutext");
		

		if(!lpvSharedMemory)
		{
			cerr << "failed to create FT_Mutext\n";
			return;
		}

		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&server::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
		);
	}

	void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
	{
		if (!error && bytes_recvd > 0)
		{
			unsigned char op = data_[0];

			float p, y, r;

			char* ptr = (char*)&y;

			for(int i = 0; i < 4; i++)
				ptr[i] = data_[1 + 3 - i];

			ptr = (char*)&p;

			for(int i = 0; i < 4; i++)
				ptr[i] = data_[1 + 4 + + 3 - i];

			ptr = (char*)&r;

			for(int i = 0; i < 4; i++)
				ptr[i] = data_[1 + 4 + 4 + + 3 - i];
			
			RawInput[GraphPos] = CAngle(p, y, r);


			lua_getglobal(g_pState, "clean");
			
			if(!lua_isfunction(g_pState, -1))
			{
				cout << "error: `clean' not function!\n";
				return;
			}

			lua_pushnumber(g_pState, p);
			lua_pushnumber(g_pState, y);
			lua_pushnumber(g_pState, r);

			if(lua_pcall(g_pState, 3, 3, 0))
			{
				cout << "error: " << lua_tostring(g_pState, -1) << "\n";
				return;
			}

			p = lua_tonumber(g_pState, -3);
			y = lua_tonumber(g_pState, -2);
			r = lua_tonumber(g_pState, -1);
			lua_pop(g_pState, 3);

			/*

			float by,bp,br;
			by = y;
			bp = p;
			br = r;

			ewma_p.Add(p);
			ewma_y.Add(y);
			ewma_r.Add(r);

			p = ewma_p.Get();
			y = ewma_y.Get();
			r = ewma_r.Get();
			
			float dy = abs(by - y);
			float dp = abs(bp - p);
			float dr = abs(br - r);
			
			const float power = 5;
#define POW(_x) (_x*50)

			dy = max(2.f, 100.0f - POW(dy));
			dp = max(2.f, 100.0f - POW(dp));
			dr = max(2.f, 100.0f - POW(dr));

			ewma_y.Set(2048, dy);
			ewma_p.Set(2048, dp);
			ewma_r.Set(2048, dr);

			cout << dy << '\t';

			*/


			/*

			
			p = PID_P.Update(p);
			y = PID_Y.Update(y);
			r = PID_R.Update(r);
			*/

			if(GetKeyState(VK_SCROLL) & 0x80)
			{
				cout << "cal\n";
				CalAng = CAngle(p, y, r);
			}

			p -= CalAng.Pitch;
			y -= CalAng.Yaw;
			r -= CalAng.Roll;
			

			

			//if(angvel > 100000 && !(GetKeyState(VK_SCROLL) & 0x80))
//				goto end;
			/*
			double cp = lastp - p;
			double cy = lasty - y;
			double cr = lastr - r;

			double vel = sqrt(cp*cp + cy*cy + cr*cr);

			lastp = p;
			lasty = y;
			lastr = r;
			
			Avg.push_back(CAngle(p, y, r));
			

			int smoothing;
			
			if(vel < 0.1)
				smoothing = 20;
			//else if(vel < 0.2)
			//	smoothing = 30;
			//else if(vel < 0.3)
			//	smoothing = 15;
			else
				smoothing = 7;

			
			//cout << smoothing << " (" << Avg.size() << ")\t";

			while(Avg.size() >  smoothing)
				Avg.pop_front();
			
			p = y = r = 0;
			for(auto it = Avg.begin(); it != Avg.end(); it++)
			{
				p += (*it).Pitch;
				y += (*it).Yaw;
				r += (*it).Roll;
			}

			p /= (float)Avg.size();
			y /= (float)Avg.size();
			r /= (float)Avg.size();
			*/
			
			lua_getglobal(g_pState, "scale");
			
			if(!lua_isfunction(g_pState, -1))
			{
				cout << "error: `scale' not function!\n";
				return;
			}

			lua_pushnumber(g_pState, p);
			lua_pushnumber(g_pState, y);
			lua_pushnumber(g_pState, r);

			if(lua_pcall(g_pState, 3, 3, 0))
			{
				cout << "error: " << lua_tostring(g_pState, -1) << "\n";
				return;
			}

			p = lua_tonumber(g_pState, -3);
			y = lua_tonumber(g_pState, -2);
			r = lua_tonumber(g_pState, -1);
			lua_pop(g_pState, 3);

			prop->m_Ang = CAngle(p, y, r);
			model.CamWidth = 320;
			model.CamHeight = 240;

			if(p >= 180.0)
				p -= 360;
			if(p <= -180.0)
				p += 360;

			if(y >= 180.0)
				y -= 360;
			if(y <= -180.0)
				y += 360;

			if(r >= 180.0)
				r -= 360;
			if(r <= -180.0)
				r += 360;
			
			model.Pitch = p	/ 180.0 * 3.1415926;
			model.Yaw	= y	/ 180.0 * 3.1415926;
			model.Roll	= r	/ 180.0 * 3.1415926;

			
			pEngineInstance->GetCamera()->SetAngle(prop->m_Ang);

			SmoothOut[GraphPos] = CAngle(p, y, r);
			GraphPos++;
			if(GraphPos >= 100)
				GraphPos = 0;
			
			bool reset = true;

#define KEY_BIND(_x_, _key1_, _key2_, _amm_) \
	else if(GetKeyState(_key1_) & 0x80)\
		_x_ += _amm_;\
	else if(GetKeyState(_key2_) & 0x80)\
		_x_ -= _amm_
			

			if(false)
				;

			KEY_BIND(kp, VK_NUMPAD4, VK_NUMPAD1, 0.01);
			KEY_BIND(ki, VK_NUMPAD5, VK_NUMPAD2, 0.01);
			KEY_BIND(kd, VK_NUMPAD6, VK_NUMPAD3, 0.01);
			
			else
				reset = false;

			if(reset)
			{
				PID_P.Initialize(kp, ki, kd, 1);
				PID_Y.Initialize(kp, ki, kd, 1);
				PID_R.Initialize(kp, ki, kd, 1);

				cout << "p: " << kp << " i: " << ki << " d: " << kd << "\n";
			}
		}
		end:
		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&server::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
		);
	}

	void handle_send_to(const boost::system::error_code& /*error*/, size_t /*bytes_sent*/)
	{
		socket_.async_receive_from(
			boost::asio::buffer(data_, max_length), sender_endpoint_,
			boost::bind(&server::handle_receive_from, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred)
		);
	}

//private:
	boost::asio::io_service& io_service_;
	udp::socket socket_;
	udp::endpoint sender_endpoint_;
	enum { max_length = 1024 };
	char data_[max_length];
	CStaticProp* prop;

	CAngle CalAng;
	CAngle Ang;
	list<CAngle> Avg;

	CEWMA ewma_p;
	CEWMA ewma_y;
	CEWMA ewma_r;

	PID_Controller PID_Y;
	PID_Controller PID_P;
	PID_Controller PID_R;

	CAngle ReTrack;
	unsigned int retrack_samples;
	bool retrack;
	float lastp, lasty, lastr;
	double lastt;

	double kp, ki, kd;

	HANDLE hFileMap;		  			// Handle to file mapping
	HANDLE hMutex;						// Handle to shared memory map mutex
	LPVOID lpvSharedMemory;		      	// Pointer to shared memory map
	TFreeTrackData model;

};

int L_include(lua_State* L)
{
	luaL_checkstring(L, -1);

	string x = lua_tostring(L, -1);
	x = pEngineInstance->GetFileSystem()->GetScript(x);

	if(luaL_dofile(L, x.c_str()))
		cout << "error: failed to include \"" << lua_tostring(L, -1) << "\"\n";
	
	return 0;
}

int L_gettime(lua_State* L)
{
	lua_pushnumber(L, pEngineInstance->GetTime());
	return 1;
}

int L_keydown(lua_State* L)
{
	int x = lua_tointeger(L, -1);
	lua_pushboolean(L, GetKeyState(x) & 0x80);

	return 1;
}

int main()
{
	
	// Set up the engine
	pEngineInstance = new CEngineInstance();
	pEngineInstance->Init();

	pEngineInstance->UpdateWindow("Head Track", 
		pEngineInstance->GetOptions()->GetSetting("graphics.resolution.x").GetValue<unsigned int>(1024),
		pEngineInstance->GetOptions()->GetSetting("graphics.resolution.y").GetValue<unsigned int>(768),
		32, false); // 0, 0 for loading from settings	
	pEngineInstance->GetCamera()->SetPosition(CVector(-1, 0, 10));
	
	
	pEngineInstance->GetOptions();

	// Load lua state
	g_pState = lua_open();
	luaJIT_setmode(g_pState, 0, LUAJIT_MODE_ON);
	luaL_openlibs(g_pState);

	lua_pushnil(g_pState);
	lua_setglobal(g_pState, "dofile");

	lua_pushcfunction(g_pState, L_include);
	lua_setglobal(g_pState, "include");

	lua_pushcfunction(g_pState, L_gettime);
	lua_setglobal(g_pState, "gettime");

	lua_pushcfunction(g_pState, L_keydown);
	lua_setglobal(g_pState, "keydown");

	string script = pEngineInstance->GetOptions()->GetSetting("script.file").GetString("default.lua");
	string func = pEngineInstance->GetOptions()->GetSetting("script.func").GetString("clean");
		
	string path = pEngineInstance->GetFileSystem()->GetScript(script);
	
	if(luaL_loadfile(g_pState, path.c_str()) || lua_pcall(g_pState, 0, 0, 0))
	{
		cout << "Script error: " << lua_tostring(g_pState, -1) << "\nPress enter to exit...";
		
		char x;
		cin >> x;
		return 0;
	}

	// Add a prop
	CStaticProp* Prop1 = new CStaticProp();
	Prop1->SetPosition(CVector(0, 0, 0));
	pEngineInstance->GetWorld()->AddEntity(Prop1);


	// Setup the connection stuff
	boost::asio::io_service io_service;
	server s(io_service, 61, Prop1);
	boost::thread t([&]
	{
		io_service.run();
	});

	boost::thread t2([&]
	{
		while(true)
		{
			//switch (WaitForSingleObject(s.hMutex, 10)) 
			{ 
				//case WAIT_OBJECT_0:
					s.model.DataID++;
					CopyMemory(s.lpvSharedMemory, &(s.model), sizeof(TFreeTrackData));
					
					//ReleaseMutex(s.hMutex);
			}
		}
	});
	
	/*
	{
		double force = 1.0;

		if(force > 1.0) 
			force = 1.0;

		force *= 2.0 * 1.5;
		CVector pos(0, 0, 0);
		CVector dir(10, 0);

		CVector motion = dir.Normal() * force;

		for(int i = 0; i < 30; i++)
		{
			pos += motion;
			motion = motion * 0.99;
			motion.Z += -0.05; // Y in minecraft!

			CStaticProp* marker = new CStaticProp();
			marker->SetPosition(pos);
			pEngineInstance->GetWorld()->AddEntity(marker);
		}
	}
	*/

	MSG msg;
	while(true)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if(msg.message == WM_QUIT)
				break;

			WndProc(msg.hwnd, msg.message, msg.wParam, msg.lParam);

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		pEngineInstance->Update();
		pEngineInstance->Draw();

		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
			glLoadIdentity();
			glOrtho(0, 100, 100, 0, 0.0f, 100.0f);
			
			glColor3ub(255, 0, 0);
			glBegin(GL_LINES);
				for(int i = 1; i < 100; i++)
				{
					glColor3ub(0, 255, 0); // yaw = green, pitch = red, roll = blue
					glVertex2d(i-1, 50.0 - RawInput[i-1].Yaw / 180.0 * 50.0);
					glVertex2d(i, 50.0 - RawInput[i].Yaw / 180.0 * 50.0);
					
					glColor3ub(0, 128, 0);
					glVertex2d(i-1, 50.0 - SmoothOut[i-1].Yaw / 180.0 * 50.0);
					glVertex2d(i, 50.0 - SmoothOut[i].Yaw / 180.0 * 50.0);

					//glVertex2d(0, 0);
					//glVertex2d(50, 50);
				}
			glEnd();

		glPopMatrix();
		

		pEngineInstance->SwapBuffer();
	}

	pEngineInstance->GetOptions()->SaveSettings();
	
	
	t.detach();
	t2.detach();
	
	delete pEngineInstance;
}