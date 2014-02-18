#include "Camera.h"
#define private public
#include "Engine.h"
#undef private

CCamera::CCamera()
{
	m_Position.X = 0.0;
	m_Position.Y = 0.0;
	m_Position.Z = 0.0;
	
	m_Up.X = 0.0;
	m_Up.Y = 0.0;
	m_Up.Z = 1.0;

	m_Angle = CAngle(0, 0, 0);

	m_VelocityAccumulator = 0.0;
	m_Velocity = CVector(0, 0, 0);
}

void CCamera::LookAt()
{
	CVector dir = m_Angle.Forward();
	dir += m_Position;

	gluLookAt(m_Position.X,		m_Position.Y,	m_Position.Z,
			  dir.X,			dir.Y,			dir.Z,
			  m_Up.X,			m_Up.Y,			m_Up.Z
	);
}

void CCamera::SetPosition(CVector& Vec)
{
	m_Position.X = Vec.X;
	m_Position.Y = Vec.Y;
	m_Position.Z = Vec.Z;
}

CVector CCamera::GetPosition()
{
	return m_Position;
}

void CCamera::SetAngle(const CAngle& Ang)
{
	m_Angle = Ang;
}

CAngle CCamera::GetAngle()
{
	return m_Angle;
}

double modulus(double a, double b)
{
	int result = static_cast<int>( a / b );
	return a - static_cast<double>( result ) * b;
}

void CCamera::Update(double FrameTime)
{
	if(!pEngineInstance->Focused())
		return;

	if(GetKeyState('L') & 0x80)
		m_Position = CVector(10, 0, 1);

	int CenterX = pEngineInstance->m_WindowWidth / 2;
	int CenterY = pEngineInstance->m_WindowHeight / 2;

	POINT mpos;
	GetCursorPos(&mpos);
	
	if(mpos.x != CenterX || mpos.y != CenterY)
	{
		double Y = CenterX - mpos.x;
		double P = CenterY - mpos.y;

		m_Angle.Pitch += P * 0.03;
		m_Angle.Yaw += Y * 0.03;

		if(m_Angle.Pitch > 89.9)
			m_Angle.Pitch = 89.9;
		else if(m_Angle.Pitch < -89.9)
			m_Angle.Pitch = -89.9;
		
		m_Angle.Yaw = modulus(m_Angle.Yaw, 360);
		

		SetCursorPos(CenterX, CenterY);
	}
	
	double right = 0.0;
	double fwd = 0.0;
	double up = 0.0;

	if(GetKeyState('D') & 0x80)
		right += 1.0;
	if(GetKeyState('A') & 0x80)
		right -= 1.0;
	
	if(GetKeyState('W') & 0x80)
		fwd += 1.0;
	if(GetKeyState('S') & 0x80)
		fwd -= 1.0;

	if(GetKeyState(' ') & 0x80)
		up += 1.0;

	if(up == 0.0 && fwd == 0.0 && right == 0.0 && m_Velocity.LengthSqr() == 0.0)
		return;

	double speed = 5.0;

	if(GetKeyState(VK_SHIFT) & 0x80)
		speed = 10.0;

	if(GetKeyState(VK_CONTROL) & 0x80)
		speed = 1.0;

	speed *= 0.05;

	CVector vFwd = m_Angle.Forward() * fwd;
	CVector vRight = m_Angle.Right() * right;
	CVector vUp = CVector(0.0, 0.0, 1.0) * up;

	CVector acceleration = vUp + vRight + vFwd;

	double accelspeed = min(acceleration.Length(), speed);
	CVector acceldir = acceleration.Normal();

	acceleration = acceldir * accelspeed * speed;

	const double NoClipAccelerate = 5.0;

	CVector newvel = m_Velocity + acceleration * FrameTime * NoClipAccelerate;
	newvel = newvel * (0.95 - FrameTime * 4.0);
	
	m_Velocity = newvel;
	m_Position = m_Position + (newvel * 1000.0) * FrameTime;

	if(GetKeyState('L') & 0x80)
		m_Position = CVector(10, 0, 1);
}