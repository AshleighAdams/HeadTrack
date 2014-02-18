#ifndef CAMERA_H
#define CAMERA_H

#include "Vector.h"

class CCamera
{
public:
	CCamera();
	void SetPosition(CVector& Vec);
	CVector GetPosition();
	void SetAngle(const CAngle& Ang);
	CAngle GetAngle();
	void Update(double FrameTime);
	void LookAt();
private:
	CAngle m_Angle;
	CVector m_Position;
	CVector m_Velocity;
	double m_VelocityAccumulator;

	CVector m_Up;
	enum MoveMode { FirstPerson, FreeLook } m_MoveMode;
};

#endif